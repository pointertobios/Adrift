// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/allocator/types.h"

#include "jungle/os/memory.h"

namespace jungle::allocator {

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

constexpr usize arena_alloc_section_size = 16 * 1024;

arena *arena::create() {
    thread_local u8 *tls_arena_current_alloc_section{nullptr};
    thread_local usize tls_arena_current_begin{arena_alloc_section_size};

    if (arena_alloc_section_size - tls_arena_current_begin < sizeof(arena)) {
        tls_arena_current_alloc_section =
            static_cast<u8 *>(os::memory::reserve_space(arena_alloc_section_size, arena_alloc_section_size));
        tls_arena_current_begin = 0;
    }

    auto res = reinterpret_cast<arena *>(tls_arena_current_alloc_section + tls_arena_current_begin);
    os::memory::commit_space(res, sizeof(arena));
    new (res) arena{};
    return res;
}

void arena::destroy() {
    this->~arena();
    if ((reinterpret_cast<usize>(this) & (arena_alloc_section_size - 1)) == 0) {
        os::memory::deprecate_space(this, arena_alloc_section_size);
    }
}

arena::arena()
        : m_start{static_cast<u8 *>(os::memory::reserve_space(size, size))}
        , m_numa_node{0} {}

arena::~arena() { os::memory::deprecate_space(const_cast<u8 *>(m_start), size); }

};  // namespace jungle::allocator
