// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "adrift/core/ecs/component.h"
#include "adrift/core/ecs/component_storage.h"
#include "adrift/core/ecs/manager.h"
#include "adrift/preusing.h"

namespace adrift::core::component {

class Motion : public ecs::Component<Motion> {
public:
    using Storage = ecs::DenseComponentStorage<Motion>;

private:
    vector3f velocity;
    vector3f acceleration;
};

adrift_core_ecs_register_component(Motion);

};  // namespace adrift::core::component
