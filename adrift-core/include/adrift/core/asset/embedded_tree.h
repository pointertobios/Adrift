// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <string_view>

namespace adrift::core::asset {

struct EmbeddedAssetNode {
    const std::string_view name;
    const std::span<const std::byte> data;
    const EmbeddedAssetNode *const next;
    const EmbeddedAssetNode *const children;
};

};  // namespace adrift::core::asset
