// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

#include "adrift/async/future.h"
#include "adrift/core/asset/asset_id.h"
#include "adrift/core/level.h"
#include "adrift/core/service/service.h"

namespace adrift::core {

class Game : public service::Service {
public:
    Game();
    ~Game() override = default;

    ustr name() const override { return "Game"; }

private:
    async::future<> run(service::ServiceController &service_ctrl) override;
};

adrift_core_service_register(Game);

};  // namespace adrift::core
