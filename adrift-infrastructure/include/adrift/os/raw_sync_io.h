// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <string_view>

namespace adrift::os {

void write_stdout(std::string_view str);
void write_stderr(std::string_view str);

};  // namespace adrift::os
