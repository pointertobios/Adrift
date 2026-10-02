// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/level.h"
#include "adrift/core/ecs/manager.h"

namespace adrift::core {

Level::Level(std::span<string_id> using_components) {
    for (auto sid : using_components) {
        auto ctor = ecs::Manager<>::get_manager_creator(sid);
        auto [tid, uptr] = ctor();
        auto res = m_managers.insert(tid, try_move(uptr));
        ADRIFT_ASSERT(res, "重复的组件类型 '{}'", uptr->name());
    }
}

};  // namespace adrift::core
