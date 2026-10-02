// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/asset/asset_id.h"

namespace adrift::core {

ustr AssetID::debug() const {
    if (!*this) {
        return ustr{"AssetID(NONE)"};
    }
    ustr result{"AssetID("};
    auto range = std::span{reinterpret_cast<const u8 *>(&m_id), sizeof(m_id)};
    result.append(util::base64_encoder_view{range});
    result.append(")");
    return result;
}

};  // namespace adrift::core
