// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/allocator/types.h"

#include <mutex>
#include <ranges>

#include "jungle/allocator/allocate.h"
#include "jungle/os/memory.h"

namespace jungle::allocator {

slice_descriptor *&slice_descriptor::local_slice_descriptor_slice_descriptor_list() {
    thread_local slice_descriptor *tls_slice_descriptor_slice_descriptor_list{nullptr};
    return tls_slice_descriptor_slice_descriptor_list;
}

slice_descriptor *&slice_descriptor::local_list(usize size_class) {
    thread_local std::array<slice_descriptor *, arena::size_classes> tls_list;
    thread_local bool tls_once_flag{true};
    if (tls_once_flag) {
        tls_once_flag = false;
        std::ranges::fill(tls_list, nullptr);
    }

    JUNGLE_ASSERT(size_class < arena::size_classes);
    return tls_list[size_class];
}

slice_descriptor::slice_descriptor(u8 *start, usize size, usize unit_size)
        : m_start{start}
        , m_size{size}
        , m_unit_size{unit_size}
        , m_unit_count{size / unit_size} {
    m_free_list = reinterpret_cast<block_list *>(const_cast<u8 *>(m_start));
    new (m_free_list) block_list{nullptr, m_unit_count};
}

slice_descriptor_slice::slice_descriptor_slice()
        : m_storage{}
        , m_descriptor{reinterpret_cast<u8 *>(&m_storage), sizeof(m_storage), sizeof(slice_descriptor)} {}

std::atomic<arena *> g_arena_list{nullptr};

union map_entry {
    std::atomic<struct arena_map *> sub_map;
    std::atomic<arena *> target;
};

struct alignas(4096) arena_map {
    map_entry entries[512]{nullptr};
};

std::array<std::atomic<arena_map *>, 32> g_arena_map{nullptr};

namespace helper {

std::array<u16, 5> into_map_indeces(void *ptr) {
    auto addr = reinterpret_cast<usize>(ptr);
    std::array<u16, 5> arr{};
    arr[0] = (addr >> 22) & 0x1ff;
    arr[1] = (addr >> (22 + 9)) & 0x1ff;
    arr[2] = (addr >> (22 + 2 * 9)) & 0x1ff;
    arr[3] = (addr >> (22 + 3 * 9)) & 0x1ff;
    arr[4] = (addr >> (22 + 4 * 9)) & 0x3f;
    return arr;
}

};  // namespace helper

void arena::map_arena(void *start, arena *target) {
    auto arr = helper::into_map_indeces(start);
    auto map0 = g_arena_map[arr[0]].load(morder::acquire);
    if (!map0) {
        map0 = new (target->allocate_slice(0)) arena_map{};
        g_arena_map[arr[0]].store(map0, morder::release);
    }
    auto map1 = map0->entries[arr[1]].sub_map.load(morder::acquire);
    if (!map1) {
        map1 = new (target->allocate_slice(0)) arena_map{};
        g_arena_map[arr[1]].store(map1, morder::release);
    }
    auto map2 = map1->entries[arr[2]].sub_map.load(morder::acquire);
    if (!map2) {
        map2 = new (target->allocate_slice(0)) arena_map{};
        g_arena_map[arr[2]].store(map2, morder::release);
    }
    auto map3 = map2->entries[arr[3]].sub_map.load(morder::acquire);
    if (!map3) {
        map3 = new (target->allocate_slice(0)) arena_map{};
        g_arena_map[arr[3]].store(map3, morder::release);
    }
    map3->entries[arr[4]].target.store(target, morder::release);
}

arena *arena::of_address(void *address) {
    auto arr = helper::into_map_indeces(address);
    auto map0 = g_arena_map[arr[0]].load(morder::acquire);
    JUNGLE_ASSERT(map0);
    auto map1 = map0->entries[arr[1]].sub_map.load(morder::acquire);
    JUNGLE_ASSERT(map1);
    auto map2 = map1->entries[arr[2]].sub_map.load(morder::acquire);
    JUNGLE_ASSERT(map2);
    auto map3 = map2->entries[arr[3]].sub_map.load(morder::acquire);
    JUNGLE_ASSERT(map3);
    auto res = map3->entries[arr[4]].target.load(morder::acquire);
    JUNGLE_ASSERT(res);
    return res;
}

arena *arena::create() {
    if (!g_arena_list.load(morder::acquire)) {
        return arena::bootstrap();
    }

    auto addr = allocate(sizeof(arena), alignof(arena));
    JUNGLE_ASSERT(addr);
    auto res = new (addr) arena{};
    map_arena(res->m_start, res);
    return res;
}

arena *arena::bootstrap() {
    arena tmp_arena{static_cast<u8 *>(os::memory::reserve_space(size, size))};

    auto sd_slice_descriptor = tmp_arena.allocate_descriptor_slice();

    auto arena_slice_addr = tmp_arena.allocate_slice(size_class_of<arena>);
    constexpr auto slice_total_size = units_of<arena> * slice_size;

    auto arena_sd_addr = sd_slice_descriptor->allocate();
    tmp_arena.set_slice_descriptor_of_address(
        reinterpret_cast<u8 *>(arena_sd_addr), units_of<arena>, sd_slice_descriptor);
    auto arena_sd = new (arena_sd_addr)
        slice_descriptor{arena_slice_addr, slice_total_size, units_of<arena> * lowest_unit};
    auto &local_list = slice_descriptor::local_list(size_class_of<arena>);
    JUNGLE_ASSERT(!local_list);
    local_list = arena_sd;

    auto arena_addr = arena_sd->allocate();
    auto res = new (arena_addr) arena(std::move(tmp_arena));

    while (true) {
        auto first = g_arena_list.load(morder::acquire);
        res->m_next.store(first, morder::release);  // 申请新 slice 的线程关心它的值
        if (g_arena_list.compare_exchange_strong(first, res, morder::acq_rel, morder::relaxed)) {
            break;
        }
    }

    map_arena(res->m_start, res);

    return res;
}

void arena::destroy() {
    auto host_arena = arena::of_address(this);
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
    auto offset = address - m_start;
    JUNGLE_ASSERT(offset >= 0 && offset < static_cast<isize>(slice_count * slice_size));
    auto slice_index = offset / slice_size;
    return m_slice_radix_map[slice_index];
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
        , m_load{rhs.m_load}
        , m_next{rhs.m_next.load(morder::relaxed)}
        , m_slice_radix_map{rhs.m_slice_radix_map} {
    rhs.m_start = nullptr;
}

arena::~arena() {
    if (m_start) {
        os::memory::deprecate_space(const_cast<u8 *>(m_start), size);
    }
}

slice_descriptor *arena::allocate_descriptor_slice() {
    auto sd_slice_addr = allocate_slice(0);
    auto sd_slice = new (sd_slice_addr) slice_descriptor_slice{};
    auto sd_slice_descriptor = &sd_slice->m_descriptor;

    auto &local_sl_sl_list = slice_descriptor::local_slice_descriptor_slice_descriptor_list();
    sd_slice_descriptor->m_next = local_sl_sl_list;
    local_sl_sl_list = sd_slice_descriptor;

    set_slice_descriptor_of_address(reinterpret_cast<u8 *>(sd_slice_descriptor), 1, sd_slice_descriptor);

    return sd_slice_descriptor;
}

void arena::set_slice_descriptor_of_address(u8 *address, usize count, slice_descriptor *sd) {
    auto offset = address - m_start;
    JUNGLE_ASSERT(offset >= 0 && offset < static_cast<isize>(slice_count * slice_size));
    auto slice_index = offset / slice_size;
    JUNGLE_ASSERT(slice_index + count <= slice_count);
    std::ranges::fill(&m_slice_radix_map[slice_index], &m_slice_radix_map[slice_index + count], sd);
}

};  // namespace jungle::allocator
