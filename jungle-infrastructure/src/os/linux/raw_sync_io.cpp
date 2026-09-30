// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/os/raw_sync_io.h"

#include <unistd.h>

namespace jungle::os {

void write_stdout(std::string_view str) {
    ::write(1, str.data(), str.size());
}

void write_stderr(std::string_view str) {
    ::write(2, str.data(), str.size());
}

};  // namespace jungle::os
