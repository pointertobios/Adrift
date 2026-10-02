// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/allocator/allocate.h"

#include <bit>

#include "adrift/allocator/types.h"
#include "adrift/os/memory.h"

namespace adrift::allocator {

thread_local std::array<block_list *, arena::size_classes> tls_fast_freelist{nullptr};

void *allocate(usize n, usize align) {
    ADRIFT_ASSERT(std::popcount(align) == 1, "不是合法的对齐值");

    const auto size_class = size_class_of(std::max(n, align));
    if (size_class >= arena::size_classes) {
        const auto ptr = os::memory::reserve_space(n, align);
        os::memory::commit_space(ptr, n);
        return ptr;
    }

    // 本地快速路径
    if (auto &freelist = tls_fast_freelist[size_class]) {
        auto res = freelist;
        if (res->unit_count == 1) [[likely]] {
            freelist = res->next;
        } else {
            auto next_addr = reinterpret_cast<u8 *>(res) + units_of_size_class(size_class) * lowest_unit;
            freelist = new (next_addr) block_list{res->next, res->unit_count - 1};
        }
        res->~block_list();
        return res;
    }

    // 本地慢速路径
    if (auto &local_slices = slice_descriptor::local_list(size_class)) {
        for (slice_descriptor *sd = local_slices, *last_sd = nullptr; sd; last_sd = sd, sd = sd->m_next) {
            if (const auto res = sd->allocate()) {
                if (const auto next = sd->m_next; next && sd->get_free_units() * 2 < next->get_free_units()) {
                    sd->m_next = next->m_next;
                    next->m_next = sd;
                    if (last_sd) {
                        last_sd->m_next = next;
                    } else {
                        local_slices = next;
                    }
                }
                return res;
            }
        }
    }

    // slice 分配
    if (const auto sd = slice_descriptor::create(size_class)) {
        return sd->allocate();
    }

    // arena 分配
    arena::create();
    const auto sd = slice_descriptor::create(size_class);
    ADRIFT_ASSERT(sd);
    return sd->allocate();
}

void deallocate(void *ptr, usize n, usize align) {
    ADRIFT_ASSERT(std::popcount(align) == 1, "不是合法的对齐值");

    const auto size_class = size_class_of(std::max(n, align));
    if (size_class >= arena::size_classes) {
        os::memory::deprecate_space(ptr, n);
        return;
    }

    auto &freelist = tls_fast_freelist[size_class];
    const auto block = new (ptr) block_list{freelist, 1};
    freelist = block;
}

};  // namespace adrift::allocator
