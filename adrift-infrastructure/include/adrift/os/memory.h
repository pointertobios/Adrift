// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "adrift/types/int.h"

namespace adrift::os::memory {

void *reserve_space(usize size, usize alignment);

void commit_space(void *address, usize size);

void uncommit_space(void *address, usize size);

void deprecate_space(void *address, usize size);

};  // namespace adrift::os::memory
