// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "adrift/types/int.h"

namespace adrift::allocator {

void *allocate(usize n, usize align);

void deallocate(void *ptr, usize n, usize align);

// deallocate 只会本地快速路径归还
// 得益于本项目用于支持游戏引擎，可以让引擎在空闲时自主选择做 cleanup
namespace cleanup {

enum class Level {
    LocalFast,
    LocalSlices,
    SliceDestroy,
};

void do_cleanup(Level cleanup_level);

};  // namespace cleanup

};  // namespace adrift::allocator
