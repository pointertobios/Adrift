// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "adrift/assert.h"
#include "adrift/panic.h"
#include "adrift/types/concepts.h"

namespace adrift {

template<concepts::deref_nullable T>
auto &unwrap(T &&value) {
    if (value) {
        return *value;
    }
    panic("unwrapped a null value.");
}

template<concepts::deref_nullable T>
auto &assert_unwrap(T &&value) {
    ADRIFT_ASSERT(value);
    return *value;
}

};  // namespace adrift
