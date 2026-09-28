// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/os/memory.h"

#ifndef NOMINMAX
#    define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace jungle::os::memory {

void *reserve_space(usize size, usize alignment) {
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
    return ::VirtualAlloc2(
        ::GetCurrentProcess(), nullptr, size, MEM_RESERVE, PAGE_NOACCESS, &parameter, 1);
}

void commit_space(void *address, usize size) {
    ::VirtualAlloc(address, size, MEM_COMMIT, PAGE_READWRITE);
}

void uncommit_space(void *address, usize size) {
    ::VirtualFree(address, size, MEM_DECOMMIT);
}

void deprecate_space(void *address, usize) {
    ::VirtualFree(address, 0, MEM_RELEASE);
}

};  // namespace jungle::os::memory
