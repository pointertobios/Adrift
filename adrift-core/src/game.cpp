// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/game.h"

#include <print>

namespace adrift::core {

Game::Game()
        : Service{type_id::of<Game>()} {}

async::future<> Game::run(service::ServiceController &service_ctrl) {
    std::println("Hello Adrift");
    service_ctrl.stop();
    co_return;
}

};  // namespace adrift::core
