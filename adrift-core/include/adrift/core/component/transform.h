// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "adrift/core/ecs/component.h"
#include "adrift/core/ecs/component_storage.h"
#include "adrift/core/ecs/manager.h"
#include "adrift/preusing.h"

namespace adrift::core::component {

class Transform : public ecs::Component<Transform> {
public:
    using Storage = ecs::DenseComponentStorage<Transform>;

private:
    vector3f position;
    vector3f rotation;
    vector3f scale;
};

adrift_core_ecs_register_component(Transform);

};  // namespace adrift::core::component
