// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <atomic>

#include "jungle/types/int.h"
#include "jungle/types/raw_storage.h"

namespace jungle::allocator {

constexpr usize lowest_unit = 16;

constexpr usize size_class_by(usize size) { return (size - 1) / 16; }

constexpr usize units_by(usize size) { return size_class_by(size) + 1; }

template<typename T>
constexpr usize size_class_of = (sizeof(T) - 1) / 16;

template<typename T>
constexpr usize units_of = size_class_of<T> + 1;

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
    friend class arena;

public:
    static slice_descriptor *&local_slice_descriptor_slice_descriptor_list();
    static slice_descriptor *&local_list(usize size_class);

    slice_descriptor(u8 *start, usize size, usize unit_size);

    u8 *allocate();
    void deallocate(void *ptr);
    void remote_deallocate(void *ptr);

private:
    const u8 *m_start;
    const usize m_size;
    const usize m_unit_size;
    const usize m_unit_count;

    block_list *m_free_list{nullptr};
    usize m_free_units{m_unit_count};

    std::atomic<block_list *> m_remote_giveback_list{nullptr};

    slice_descriptor *m_next{nullptr};
};

struct slice_descriptor_slice {
    raw_storage<slice_descriptor> m_storage[63];
    slice_descriptor m_descriptor;

    slice_descriptor_slice();
};

class arena {
public:
    static constexpr usize slice_size = 0x1'000;

private:
    static constexpr usize size = 0x400'000;

    static constexpr usize slice_count = size / slice_size;

public:
    static constexpr usize size_classes = size / slice_size;

    static arena *of_address(void *address);
    static arena *create();
    static arena *bootstrap();

    void destroy();

    slice_descriptor *slice_descriptor_of_address(u8 *address) const;

    u8 *allocate_slice(usize size_class);
    void deallocate_slice(slice_descriptor *sd);

private:
    arena();
    explicit arena(u8 *start);

    arena(arena &&rhs);

    ~arena();

    slice_descriptor *allocate_descriptor_slice();

    void set_slice_descriptor_of_address(u8 *address, usize count, slice_descriptor *sd);

    static void map_arena(void *start, arena *target);

    u8 *m_start;
    const u8 m_numa_node;

    u16 m_load{0};

    std::atomic<arena *> m_next{nullptr};

    std::array<slice_descriptor *, slice_count> m_slice_radix_map{0};
};

};  // namespace jungle::allocator
