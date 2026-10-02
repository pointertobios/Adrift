// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <format>
#include <string>

#include "adrift/panic.h"

namespace adrift {

#ifdef ADRIFT_DEBUG_ENABLED
#    define ADRIFT_ASSERT(expr, ...)                                     \
        do {                                                             \
            if (!(expr)) {                                               \
                std::string lint;                                        \
                __VA_OPT__(lint = ": " + std::format(__VA_ARGS__);)      \
                ::adrift::panic("'{}' assertion failed{}", #expr, lint); \
            }                                                            \
        } while (false)
#else
#    define ADRIFT_ASSERT(expr, ...) ((void)(expr))
#endif

};  // namespace adrift