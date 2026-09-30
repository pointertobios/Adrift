// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/os/memory.h"

#include <bit>

#ifndef NOMINMAX
#    define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "jungle/assert.h"

namespace jungle::os::memory {

const auto page_size = static_cast<usize>([] {
    ::SYSTEM_INFO info{};
    ::GetSystemInfo(&info);
    return info.dwPageSize;
}());

const auto allocation_granularity = static_cast<usize>([] {
    ::SYSTEM_INFO info{};
    ::GetSystemInfo(&info);
    return info.dwAllocationGranularity;
}());

void *reserve_space(usize size, usize alignment) {
    JUNGLE_ASSERT(alignment >= allocation_granularity && std::popcount(alignment) == 1);

    ::MEM_ADDRESS_REQUIREMENTS requirements{
        .LowestStartingAddress = nullptr,
        .HighestEndingAddress = nullptr,
        .Alignment = alignment,
    };
    ::MEM_EXTENDED_PARAMETER parameter{
        .Type = MemExtendedParameterAddressRequirements,
        .Reserved = 0,
        .Pointer = &requirements,
    };
    return ::VirtualAlloc2(::GetCurrentProcess(), nullptr, size, MEM_RESERVE, PAGE_NOACCESS, &parameter, 1);
}

void commit_space(void *address, usize size) {
    JUNGLE_ASSERT((reinterpret_cast<usize>(address) & (page_size - 1)) == 0);

    ::VirtualAlloc(address, size, MEM_COMMIT, PAGE_READWRITE);
}

void uncommit_space(void *address, usize size) {
    JUNGLE_ASSERT((reinterpret_cast<usize>(address) & (page_size - 1)) == 0);

    ::VirtualFree(address, size, MEM_DECOMMIT);
}

void deprecate_space(void *address, usize) {
    JUNGLE_ASSERT((reinterpret_cast<usize>(address) & (page_size - 1)) == 0);

    ::VirtualFree(address, 0, MEM_RELEASE);
}

};  // namespace jungle::os::memory
