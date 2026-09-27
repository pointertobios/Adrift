// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/os/memory.h"

#include <sys/mman.h>

namespace jungle::os::memory {

void *reserve_space(usize size) {
    return ::mmap64(nullptr, size, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

void commit_space(void *address, usize size) {
    ::mprotect(address, size, PROT_READ | PROT_WRITE);
}

void uncommit_space(void *address, usize size) {
    ::munmap(address, size);
    ::mmap64(address, size, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

};  // namespace jungle::os::memory
