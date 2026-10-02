// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/allocator/types.h"

#include <bit>
#include <mutex>
#include <ranges>

#include "jungle/allocator/allocate.h"
#include "jungle/os/memory.h"
#include "jungle/util/unwrap.h"

namespace jungle::allocator {

namespace {

std::atomic<arena *> g_arena_list{nullptr};

void add_arena(arena *a) {
    while (true) {
        auto first = g_arena_list.load(morder::acquire);
        a->m_next.store(first, morder::release);  // 申请新 slice 的线程关心它的值
        if (g_arena_list.compare_exchange_strong(first, a, morder::acq_rel, morder::relaxed)) {
            break;
        }
    }
}

union map_entry {
    std::atomic<struct arena_map *> sub_map;
    std::atomic<arena *> target;
};

struct alignas(4096) arena_map {
    map_entry entries[512]{nullptr};
};

std::array<std::atomic<arena_map *>, 64> g_arena_map{nullptr};

std::array<u16, 5> into_map_indices(const void *ptr) {
    const auto addr = reinterpret_cast<usize>(ptr);
    std::array<u16, 5> arr{};
    arr[0] = (addr >> 22) & 0x1ff;
    arr[1] = (addr >> (22 + 9)) & 0x1ff;
    arr[2] = (addr >> (22 + 2 * 9)) & 0x1ff;
    arr[3] = (addr >> (22 + 3 * 9)) & 0x1ff;
    arr[4] = (addr >> (22 + 4 * 9)) & 0x3f;
    return arr;
}

// 尽力而为地把 next 换到 cur 之前（prev 是 cur 的前驱，nullptr 表示 cur 是表头）。
// 必须先抢下前驱链接再改 next->m_next：反过来会在中间态形成 cur <-> next 的环。
// 任一步 CAS 失败即放弃本轮调整，绝不阻塞。
bool try_promote_next_arena(arena *prev, arena *cur, arena *next) {
    const auto next_next = next->m_next.load(morder::acquire);

    // CAS 失败说明链表已被其它线程改动。
    auto &link = prev ? prev->m_next : g_arena_list;
    if (!link.compare_exchange_strong(cur, next, morder::acq_rel, morder::acquire)) {
        return false;
    }

    // 这两步 store 不会失败，因此 cur 的短暂不可达必定会被修复。
    cur->m_next.store(next_next, morder::release);
    next->m_next.store(cur, morder::release);
    return true;
}

// 沿 g_arena_list 从 start 起遍历并逐个尝试 try_arena(*arena)，命中即返回该 arena，否则返回 nullptr。
// prev 是 start 的前驱，返回时是命中 arena 的前驱（换位的 CAS 需要它）。
// stop_before 非空时遇到即停，供「走完整圈」时避免重复尝试。
arena *traverse_arenas(arena *&prev, arena *start, arena *stop_before, auto &&try_arena) {
    auto cur = start;
    while (cur && cur != stop_before) {
        if (try_arena(*cur)) {
            return cur;
        }

        const auto next = cur->m_next.load(morder::acquire);
        // 空闲 slice 富余的 arena 提前，让后续分配更早命中连续区间。
        if (next && cur->get_free_slices() * 2 < next->get_free_slices()
            && try_promote_next_arena(prev, cur, next)) {
            // 换位后 next 位于 cur 之前且尚未尝试，其前驱仍是 prev。
            cur = next;
            continue;
        }

        prev = cur;
        cur = next;
    }
    return nullptr;
}

};  // namespace

slice_descriptor *&slice_descriptor::local_slice_descriptor_slice_descriptor_list() {
    thread_local slice_descriptor *tls_slice_descriptor_slice_descriptor_list{nullptr};
    return tls_slice_descriptor_slice_descriptor_list;
}

slice_descriptor *&slice_descriptor::local_list(usize size_class) {
    thread_local std::array<slice_descriptor *, arena::size_classes> tls_list{nullptr};

    JUNGLE_ASSERT(size_class < arena::size_classes);
    return tls_list[size_class];
}

slice_descriptor *slice_descriptor::create(usize size_class) {
    u8 *addr{nullptr};
    arena *prev{nullptr};

    arena *current_arena = traverse_arenas(prev, g_arena_list.load(morder::acquire), nullptr, [&](arena &a) {
        addr = a.allocate_slice(size_class);
        return addr != nullptr;
    });

    if (!current_arena) {
        current_arena = arena::bootstrap();
        prev = nullptr;  // bootstrap 把新 arena 插到表头，其前驱为空
        addr = current_arena->allocate_slice(size_class);
    }
    JUNGLE_ASSERT(addr);

    auto sd_sd = local_slice_descriptor_slice_descriptor_list();
    if (!sd_sd) {
        (void)traverse_arenas(prev, current_arena, nullptr, [&](arena &a) {
            sd_sd = a.allocate_slice_descriptor_slice();
            return sd_sd != nullptr;
        });

        // 回到表头补完整圈，到 current_arena 即停以免重复尝试。
        if (!sd_sd) {
            arena *wrap_prev{nullptr};
            (void)traverse_arenas(
                wrap_prev, g_arena_list.load(morder::acquire), current_arena, [&](arena &a) {
                    sd_sd = a.allocate_slice_descriptor_slice();
                    return sd_sd != nullptr;
                });
        }

        if (!sd_sd) {
            sd_sd = arena::bootstrap()->allocate_slice_descriptor_slice();
        }

        sd_sd = local_slice_descriptor_slice_descriptor_list();
    }

    slice_descriptor *last_sd{nullptr};
    while (true) {
        JUNGLE_ASSERT(sd_sd);

        const auto next = sd_sd->m_next;
        if (auto sd_addr = sd_sd->allocate()) {
            if (next && sd_sd->get_free_units() * 2 < next->get_free_units()) {
                sd_sd->m_next = next->m_next;
                next->m_next = sd_sd;
                if (last_sd) {
                    last_sd->m_next = next;
                } else {
                    local_slice_descriptor_slice_descriptor_list() = next;
                }
            }

            const auto sd = new (sd_addr) slice_descriptor{
                addr, units_of_size_class(size_class) * arena::slice_size,
                units_of_size_class(size_class) * lowest_unit};
            current_arena->set_slice_descriptor_of_address(addr, units_of_size_class(size_class), sd);

            auto &list = local_list(size_class);
            sd->m_next = list;
            list = sd;

            return sd;
        }

        last_sd = sd_sd;
        sd_sd = sd_sd->m_next;
    }
}

void slice_descriptor::destroy() {
    const auto size_class = size_class_of(m_unit_size);
    auto &a = assert_unwrap(arena::of_address(m_start));
    a.set_slice_descriptor_of_address(m_start, (m_size + arena::slice_size - 1) / arena::slice_size, nullptr);
    a.deallocate_slice(const_cast<u8 *>(m_start), size_class);

    auto sd_sd =
        assert_unwrap(arena::of_address(this)).slice_descriptor_of_address(reinterpret_cast<u8 *>(this));
    assert_unwrap(sd_sd).deallocate(this);
}

slice_descriptor::slice_descriptor(u8 *start, usize size, usize unit_size)
        : m_start{start}
        , m_size{size}
        , m_unit_size{unit_size}
        , m_unit_count{size / unit_size} {
    JUNGLE_ASSERT(!(reinterpret_cast<usize>(start) & (arena::slice_size - 1)));
    JUNGLE_ASSERT(size / arena::slice_size <= arena::size_classes);
    JUNGLE_ASSERT(!(unit_size & (lowest_unit - 1)));

    m_free_list = reinterpret_cast<block_list *>(const_cast<u8 *>(m_start));
    new (m_free_list) block_list{nullptr, m_unit_count};
}

u8 *slice_descriptor::allocate() {
    const auto res = m_free_list;
    if (!res) {
        return nullptr;
    }
    if (res->unit_count == 1) [[likely]] {
        m_free_list = res->next;
    } else {
        auto next_addr = reinterpret_cast<u8 *>(res) + m_unit_size;
        m_free_list = new (next_addr) block_list{res->next, res->unit_count - 1};
    }
    res->~block_list();
    m_free_units--;
    return reinterpret_cast<u8 *>(res);
}

void slice_descriptor::deallocate(void *ptr) {
    const auto block = new (ptr) block_list{m_free_list, 1};
    m_free_list = block;
    m_free_units++;
}

void slice_descriptor::remote_deallocate(void *ptr) {
    const auto block = new (ptr) block_list{m_free_list, 1};
    while (true) {
        auto first = m_remote_giveback_list.load(morder::acquire);
        block->next = first;
        if (m_remote_giveback_list.compare_exchange_strong(first, block, morder::acq_rel, morder::relaxed)) {
            break;
        }
    }
}

slice_descriptor_slice::slice_descriptor_slice()
        : m_storage{}
        , m_descriptor{reinterpret_cast<u8 *>(&m_storage), sizeof(m_storage), sizeof(slice_descriptor)} {}

void arena::map_arena(void *start, arena *target) {
    const auto arr = into_map_indices(start);
    auto map0 = g_arena_map[arr[4]].load(morder::acquire);
    if (!map0) {
        map0 = new (target->allocate_slice(0)) arena_map{};
        g_arena_map[arr[4]].store(map0, morder::release);
    }
    auto map1 = map0->entries[arr[3]].sub_map.load(morder::acquire);
    if (!map1) {
        map1 = new (target->allocate_slice(0)) arena_map{};
        map0->entries[arr[3]].sub_map.store(map1, morder::release);
    }
    auto map2 = map1->entries[arr[2]].sub_map.load(morder::acquire);
    if (!map2) {
        map2 = new (target->allocate_slice(0)) arena_map{};
        map1->entries[arr[2]].sub_map.store(map2, morder::release);
    }
    auto map3 = map2->entries[arr[1]].sub_map.load(morder::acquire);
    if (!map3) {
        map3 = new (target->allocate_slice(0)) arena_map{};
        map2->entries[arr[1]].sub_map.store(map3, morder::release);
    }
    map3->entries[arr[0]].target.store(target, morder::release);
}

arena *arena::of_address(const void *address) {
    const auto arr = into_map_indices(address);
    const auto map0 = g_arena_map[arr[4]].load(morder::acquire);
    JUNGLE_ASSERT(map0);
    const auto map1 = map0->entries[arr[3]].sub_map.load(morder::acquire);
    JUNGLE_ASSERT(map1);
    const auto map2 = map1->entries[arr[2]].sub_map.load(morder::acquire);
    JUNGLE_ASSERT(map2);
    const auto map3 = map2->entries[arr[1]].sub_map.load(morder::acquire);
    JUNGLE_ASSERT(map3);
    const auto res = map3->entries[arr[0]].target.load(morder::acquire);
    JUNGLE_ASSERT(res);
    return res;
}

arena *arena::create() {
    auto addr = allocate(sizeof(arena), alignof(arena));
    if (!addr) {
        panic_oom();
    }
    const auto res = new (addr) arena{};

    add_arena(res);

    map_arena(res->m_start, res);

    return res;
}

arena *arena::bootstrap() {
    arena tmp_arena{static_cast<u8 *>(os::memory::reserve_space(size, size))};

    const auto sd_slice_descriptor = tmp_arena.allocate_slice_descriptor_slice();

    const auto arena_slice_addr = tmp_arena.allocate_slice(size_class_of_v<arena>);
    constexpr auto slice_total_size = units_of_v<arena> * slice_size;

    auto arena_sd_addr = sd_slice_descriptor->allocate();
    const auto arena_sd = new (arena_sd_addr)
        slice_descriptor{arena_slice_addr, slice_total_size, units_of_v<arena> * lowest_unit};
    tmp_arena.set_slice_descriptor_of_address(arena_slice_addr, units_of_v<arena>, arena_sd);

    auto &local_list = slice_descriptor::local_list(size_class_of_v<arena>);
    arena_sd->m_next = local_list;
    local_list = arena_sd;

    auto arena_addr = arena_sd->allocate();
    const auto res = new (arena_addr) arena(std::move(tmp_arena));

    add_arena(res);

    map_arena(res->m_start, res);

    return res;
}

void arena::destroy() {
    const auto host_arena = arena::of_address(this);
    auto sd = host_arena->slice_descriptor_of_address(reinterpret_cast<u8 *>(this));
    sd->deallocate(this);
    if (host_arena == this) {
        sd->~slice_descriptor();
        auto sd_sd = host_arena->slice_descriptor_of_address(reinterpret_cast<u8 *>(sd));
        sd_sd->deallocate(sd);
        sd_sd->~slice_descriptor();
    }
    this->~arena();
}

slice_descriptor *arena::slice_descriptor_of_address(u8 *address) const {
    const auto offset = address - m_start;
    JUNGLE_ASSERT(offset >= 0 && offset < static_cast<isize>(slice_count * slice_size));
    const auto slice_index = offset / slice_size;
    return m_slice_radix_map[slice_index];
}

u8 *arena::allocate_slice(usize size_class) {
    auto for_each_zero_run = [](auto &bitmap, auto &&f) {
        usize run_start = 0;
        usize run_len = 0;

        for (usize i = 0; i < bitmap.size(); ++i) {
            const auto v = bitmap[i];
            if (v == ~u64{0}) {
                continue;
            }

            const auto base = i << 6;
            const auto starts = ~v & ((v << 1) | 1ull);
            const auto ends = ~v & ((v >> 1) | (1ull << 63));

            auto s_mask = starts, e_mask = ends;
            while (s_mask != 0) {
                const auto abs_s = base + std::countr_zero(s_mask);
                const auto abs_e = base + std::countr_zero(e_mask);

                if (run_len != 0 && abs_s == run_start + run_len) {
                    run_len += abs_e - abs_s + 1;
                } else {
                    if (run_len != 0) {
                        if (!f(run_start, run_len)) {
                            return;
                        }
                    }
                    run_start = abs_s;
                    run_len = abs_e - abs_s + 1;
                }
                s_mask &= s_mask - 1;
                e_mask &= e_mask - 1;
            }

            if (run_len != 0 && run_start + run_len != base + 64) {
                if (!f(run_start, run_len)) {
                    return;
                }
                run_len = 0;
            }
        }

        if (run_len != 0) {
            f(run_start, run_len);
        }
    };

    const auto slices = units_of_size_class(size_class);
    JUNGLE_ASSERT(slices <= slice_count);
    u8 *res{nullptr};
    auto guard = m_slices_bitmap.lock();
    for_each_zero_run(*guard, [&](usize start, usize len) {
        if (len >= slices) {
            for (usize i = start; i < start + slices; ++i) {
                (*guard)[i >> 6] |= u64{1} << (i & 63);
            }

            // 已持有 m_slices_bitmap 锁，故计数自身的读可用 relaxed
            JUNGLE_ASSERT(m_free_slices.load(morder::relaxed) >= slices, "空闲 slice 计数不足");
            // release：令 get_free_slices 的 acquire 读能观察到该计数对应的位图/内存状态
            m_free_slices.fetch_sub(static_cast<u16>(slices), morder::release);

            res = m_start + start * slice_size;
            os::memory::commit_space(res, slices * slice_size);
            return false;
        }
        return true;
    });
    return res;
}

void arena::deallocate_slice(void *addr, usize size_class) {
    const auto offset_ = static_cast<u8 *>(addr) - m_start;
    JUNGLE_ASSERT(offset_ >= 0 && offset_ < static_cast<isize>(size));
    const auto slices = units_of_size_class(size_class);
    JUNGLE_ASSERT(slices <= slice_count);

    const auto offset = static_cast<usize>(offset_);
    JUNGLE_ASSERT(!(offset % slice_size));
    const auto start = offset / slice_size;
    JUNGLE_ASSERT(start + slices <= slice_count);

    os::memory::uncommit_space(addr, slices * slice_size);

    auto guard = m_slices_bitmap.lock();
    for (usize i = start; i < start + slices; ++i) {
        JUNGLE_ASSERT((*guard)[i >> 6] & (u64{1} << (i & 63)), "归还了没有被分配的 slice");
        (*guard)[i >> 6] &= ~(u64{1} << (i & 63));
    }

    JUNGLE_ASSERT(m_free_slices.load(morder::relaxed) + slices <= slice_count, "归还后空闲 slice 计数越界");
    // release：令 get_free_slices 的 acquire 读能观察到该计数对应的位图/内存状态
    m_free_slices.fetch_add(static_cast<u16>(slices), morder::release);
}

arena::arena()
        : m_start{static_cast<u8 *>(os::memory::reserve_space(size, size))}
        , m_numa_node{0} {}

arena::arena(u8 *start)
        : m_start{start}
        , m_numa_node{0} {}

arena::arena(arena &&rhs)
        : m_start{rhs.m_start}
        , m_numa_node{rhs.m_numa_node}
        , m_free_slices{rhs.m_free_slices.load(morder::relaxed)}
        , m_next{rhs.m_next.load(morder::relaxed)}
        , m_slices_bitmap{*rhs.m_slices_bitmap.lock()} {
    std::ranges::for_each(std::views::zip(m_slice_radix_map, rhs.m_slice_radix_map), [](auto &&p) {
        auto &[l, r] = p;
        l.store(r.load(morder::relaxed), morder::relaxed);
    });
    rhs.m_start = nullptr;
}

arena::~arena() {
    if (m_start) {
        os::memory::deprecate_space(m_start, size);
    }
}

slice_descriptor *arena::allocate_slice_descriptor_slice() {
    auto sd_slice_addr = allocate_slice(0);
    if (!sd_slice_addr) {
        return nullptr;
    }
    const auto sd_slice = new (sd_slice_addr) slice_descriptor_slice{};
    const auto sd_slice_descriptor = &sd_slice->m_descriptor;

    auto &local_sl_sl_list = slice_descriptor::local_slice_descriptor_slice_descriptor_list();
    sd_slice_descriptor->m_next = local_sl_sl_list;
    local_sl_sl_list = sd_slice_descriptor;

    set_slice_descriptor_of_address(reinterpret_cast<u8 *>(sd_slice_descriptor), 1, sd_slice_descriptor);

    return sd_slice_descriptor;
}

void arena::deallocate_slice_descriptor_slice(slice_descriptor *sd) {
    JUNGLE_ASSERT(
        sd->get_free_units() == sd->m_unit_count, "仍有 slice_descriptor 存活，不能回收 descriptor slice");

    const auto slice_addr = const_cast<u8 *>(sd->m_start);

    set_slice_descriptor_of_address(reinterpret_cast<const u8 *>(sd), 1, nullptr);
    sd->~slice_descriptor();

    deallocate_slice(slice_addr, 0);
}

void arena::set_slice_descriptor_of_address(const u8 *address, usize count, slice_descriptor *sd) {
    const auto offset = address - m_start;
    JUNGLE_ASSERT(offset >= 0 && offset < static_cast<isize>(slice_count * slice_size));
    const auto slice_index = offset / slice_size;
    JUNGLE_ASSERT(slice_index + count <= slice_count);
    std::ranges::fill(&m_slice_radix_map[slice_index], &m_slice_radix_map[slice_index + count], sd);
}

};  // namespace jungle::allocator
