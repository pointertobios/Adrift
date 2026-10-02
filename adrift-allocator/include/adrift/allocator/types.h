// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <atomic>

#include "adrift/sync/spinlock.h"
#include "adrift/types/int.h"
#include "adrift/types/raw_storage.h"

namespace adrift::allocator {

constexpr usize lowest_unit = 16;

constexpr usize size_class_of(usize size) { return (size - 1) / 16; }

constexpr usize units_of(usize size) { return size_class_of(size) + 1; }

constexpr usize units_of_size_class(usize size_class) { return size_class + 1; }

constexpr usize size_class_of_units(usize units) { return units - 1; }

template<typename T>
constexpr usize size_class_of_v = (sizeof(T) - 1) / 16;

template<typename T>
constexpr usize units_of_v = size_class_of_v<T> + 1;

struct block_list {
    block_list *next;
    usize unit_count;
};

class alignas(64) slice_descriptor {
    friend class arena;
    friend struct slice_descriptor_slice;

public:
    static slice_descriptor *&local_slice_descriptor_slice_descriptor_list();
    static slice_descriptor *&local_list(usize size_class);

    static slice_descriptor *create(usize size_class);
    void destroy();

    u8 *allocate();
    void deallocate(void *ptr);
    void remote_deallocate(void *ptr);

    usize get_free_units() const { return m_free_units; }

private:
    slice_descriptor(u8 *start, usize size, usize unit_size);

    const u8 *m_start;
    const usize m_size;
    const usize m_unit_size;
    const usize m_unit_count;

    block_list *m_free_list{nullptr};
    usize m_free_units{m_unit_count};

    std::atomic<block_list *> m_remote_giveback_list{nullptr};

public:
    slice_descriptor *m_next{nullptr};
};

struct slice_descriptor_slice {
    raw_storage<slice_descriptor> m_storage[63];
    slice_descriptor m_descriptor;

    slice_descriptor_slice();
};

class arena {
    friend class slice_descriptor;

public:
    static constexpr usize slice_size = 0x1'000;

private:
    static constexpr usize size = 0x400'000;

    static constexpr usize slice_count = size / slice_size;

public:
    static constexpr usize size_classes = size / slice_size;

    static arena *of_address(const void *address);
    static arena *create();
    static arena *bootstrap();

    void destroy();

    slice_descriptor *slice_descriptor_of_address(u8 *address) const;

    u8 *allocate_slice(usize size_class);
    void deallocate_slice(void *addr, usize size_class);

    u16 get_free_slices() const { return m_free_slices.load(morder::acquire); }

private:
    arena();
    explicit arena(u8 *start);

    arena(arena &&rhs);

    ~arena();

    slice_descriptor *allocate_slice_descriptor_slice();

    // 仅能释放已经不存在于 slice_descriptor::local_slice_descriptor_slice_descriptor_list() 链表内的 sd
    void deallocate_slice_descriptor_slice(slice_descriptor *sd);

    void set_slice_descriptor_of_address(const u8 *address, usize count, slice_descriptor *sd);

    static void map_arena(void *start, arena *target);

    u8 *m_start;
    const u8 m_numa_node;

    std::atomic<u16> m_free_slices{slice_count};

public:
    std::atomic<arena *> m_next{nullptr};

private:
    std::array<std::atomic<slice_descriptor *>, slice_count> m_slice_radix_map{nullptr};

    sync::spinlock<std::array<u64, slice_count / 64>> m_slices_bitmap;
};

};  // namespace adrift::allocator
