// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/os/memory.h"

#include <bit>

#include <sys/mman.h>
#include <unistd.h>

#include "jungle/assert.h"

namespace jungle::os::memory {

const auto page_size = static_cast<usize>(::sysconf(_SC_PAGESIZE));

void *reserve_space(usize size, usize alignment) {
    JUNGLE_ASSERT(alignment >= page_size && std::popcount(alignment) == 1);

    const auto reservation_size = size + alignment;
    auto *const reservation =
        ::mmap64(nullptr, reservation_size, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (reservation == MAP_FAILED) {
        return reservation;
    }

    const auto address = reinterpret_cast<usize>(reservation);
    const auto aligned_address = (address + alignment - 1) & ~(alignment - 1);
    const auto prefix_size = aligned_address - address;
    const auto suffix_size = reservation_size - prefix_size - size;

    ::munmap(reservation, prefix_size);
    ::munmap(reinterpret_cast<void *>(aligned_address + size), suffix_size);
    return reinterpret_cast<void *>(aligned_address);
}

void commit_space(void *address, usize size) {
    JUNGLE_ASSERT((reinterpret_cast<usize>(address) & (page_size - 1)) == 0);

    const auto address_value = reinterpret_cast<usize>(address);
    const auto page_address = address_value & ~(page_size - 1);
    const auto page_offset = address_value - page_address;
    ::mprotect(reinterpret_cast<void *>(page_address), page_offset + size, PROT_READ | PROT_WRITE);
}

void uncommit_space(void *address, usize size) {
    JUNGLE_ASSERT((reinterpret_cast<usize>(address) & (page_size - 1)) == 0);

    ::mmap64(address, size, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
}

void deprecate_space(void *address, usize size) {
    JUNGLE_ASSERT((reinterpret_cast<usize>(address) & (page_size - 1)) == 0);

    ::munmap(address, size);
}

};  // namespace jungle::os::memory
