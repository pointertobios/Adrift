// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "jungle/assert.h"
#include "jungle/panic.h"
#include "jungle/types/concepts.h"

namespace jungle {

template<concepts::deref_nullable T>
auto &unwrap(T &&value) {
    if (value) {
        return *value;
    }
    panic("unwrapped a null value.");
}

template<concepts::deref_nullable T>
auto &assert_unwrap(T &&value) {
    JUNGLE_ASSERT(value);
    return *value;
}

};  // namespace jungle
