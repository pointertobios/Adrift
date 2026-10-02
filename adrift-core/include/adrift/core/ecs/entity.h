// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include "adrift/assert.h"
#include "adrift/debug.h"
#include "adrift/preusing.h"

namespace adrift::core::ecs {

class Entity final {
    friend struct std::hash<Entity>;

    static constexpr u64 INVALID = 0;

public:
    constexpr Entity() = default;
    constexpr Entity(const Entity &entity) = default;
    constexpr Entity &operator=(const Entity &other) = default;
    constexpr Entity(Entity &&entity) = default;
    constexpr Entity &operator=(Entity &&other) = default;

    constexpr operator bool() const { return m_id != INVALID; }
    constexpr bool operator==(const Entity &other) const { return m_id == other.m_id; }

    constexpr Entity(u64 id)
            : m_id{id} {}

    ustr debug() const;

private:
    u64 m_id{INVALID};
};

};  // namespace adrift::core::ecs

template<>
struct std::hash<adrift::core::ecs::Entity> {
    std::size_t operator()(const adrift::core::ecs::Entity &entity) const {
        ADRIFT_ASSERT(entity);
        return entity.m_id;
    }
};
