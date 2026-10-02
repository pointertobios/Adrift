// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <string_view>

#include "adrift/types/int.h"

namespace adrift {

u128 build_id();
std::string_view build_id_string();

};  // namespace adrift
