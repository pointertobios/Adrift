// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <atomic>

#include "jungle/types/int.h"
#include "jungle/types/raw_storage.h"

namespace jungle::allocator {

struct block_list {
    block_list *next;
    usize unit_count;

    template<typename T>
    static block_list *from_address(T *address) {
        return reinterpret_cast<block_list *>(address);
    }
};

class arena;

class alignas(64) slice_descriptor {
public:
    slice_descriptor(u8 *start, usize size, usize unit_size);

private:
    const u8 *m_start;
    const usize m_size;
    const usize m_unit_size;
    const usize m_unit_count;

    block_list *m_free_list{nullptr};

    std::atomic<block_list *> m_external_giveback_list{nullptr};
};

struct slice_descriptor_slice {
    raw_storage<slice_descriptor> m_storage[63];
    slice_descriptor m_descriptor;

    slice_descriptor_slice();
};

class arena {
    static constexpr usize slice_size = 0x1'000;

    static constexpr usize size = 0x400'000;

    static constexpr usize slice_count = size / slice_size;

public:
    static arena *create();

    void destroy();

private:
    arena();
    ~arena();

    const u8 *m_start;
    const u8 m_numa_node;

    u16 m_load{0};
    
    std::atomic<arena *> m_next{nullptr};

    std::array<u16, 1024> m_slice_radix_map{0};

    block_list *m_slice_descriptor_free_list{nullptr};
};

};  // namespace jungle::allocator
