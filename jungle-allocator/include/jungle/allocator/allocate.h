// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "jungle/types/int.h"

namespace jungle::allocator {

void *allocate(usize n, usize align);

void deallocate(void *ptr, usize n, usize align);

namespace cleanup {

enum class Level {
    LocalFast,
    LocalSlices,
    SliceDestroy,
};

void do_cleanup(Level cleanup_level);

};  // namespace cleanup

};  // namespace jungle::allocator
