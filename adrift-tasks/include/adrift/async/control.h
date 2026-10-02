// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>

namespace adrift::async::control {

enum class future_state {
    empty,
    non_complete,
    awaited,
    complete,
};

};  // namespace adrift::async::control
