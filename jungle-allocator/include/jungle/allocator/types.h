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

public:
    arena(u8 *start, usize size, usize numa);

private:
    const u8 *m_start;   // `slice_size` 对齐
    const usize m_size;  // `slice_size` 对齐
    const usize m_numa_node;

    std::atomic<arena *> m_next{nullptr};

    usize m_slice_count;

    std::atomic<u8> m_bitmap_1024sl{0};
    std::atomic<u8> m_bitmap_512sl{0};
    std::atomic<u8> m_bitmap_256sl{0};
    std::atomic<u8> m_bitmap_128sl{0};
    std::atomic<u16> m_bitmap_64sl{0};
    std::atomic<u32> m_bitmap_32sl{0};
    std::atomic<u64> m_bitmap_16sl{0};
    std::array<std::atomic<u64>, 2> m_bitmap_8sl{0};
    std::array<std::atomic<u64>, 4> m_bitmap_4sl{0};
    std::array<std::atomic<u64>, 8> m_bitmap_2sl{0};
    std::array<std::atomic<u64>, 16> m_bitmap_1sl{0};
};

};  // namespace jungle::allocator
