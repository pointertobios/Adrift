// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/asset/asset.h"

#include <print>

namespace adrift::core::asset {

Asset::Asset()
        : service::Service{type_id::of<Asset>()} {}

async::future<> Asset::run(service::ServiceController &service_ctrl) {
    std::println("Asset Service");
    co_await service_ctrl.wait_for_stop();
    co_return;
}

};  // namespace adrift::core::asset
