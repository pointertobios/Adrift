// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/log/logging.h"

#include <print>

namespace adrift::core::log {

Logging::Logging()
        : service::Service{type_id::of<Logging>()} {}

async::future<> Logging::run(service::ServiceController &service_ctrl) {
    std::println("Logging Service");
    co_await service_ctrl.wait_for_stop();
    co_return;
}

};  // namespace adrift::core::log
