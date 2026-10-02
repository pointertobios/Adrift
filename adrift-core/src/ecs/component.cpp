// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/ecs/component.h"

#include <span>

namespace adrift::core::ecs {

ustr ComponentID::debug() const {
    if (!*this) {
        return ustr{"ComponentID(NONE)"};
    }
    ustr result{"ComponentID("};
    auto range = std::span{reinterpret_cast<const u8 *>(&m_id), sizeof(m_id)};
    result.append(util::base64_encoder_view{range});
    result.append(")");
    return result;
}

};  // namespace adrift::core::ecs
