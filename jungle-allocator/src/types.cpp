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

arena::arena(u8 *start, usize size, usize numa)
        : m_start{start}
        , m_size{size}
        , m_numa_node{numa}
        , m_slice_count{size / slice_size} {}

};  // namespace jungle::allocator
