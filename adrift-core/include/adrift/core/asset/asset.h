// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "adrift/core/service/service.h"

namespace adrift::core::asset {

class Asset : public service::Service {
public:
    Asset();

    ustr name() const override { return "Asset"; }

private:
    async::future<> run(service::ServiceController &service_ctrl) override;
};

adrift_core_service_register(Asset);

};  // namespace adrift::core::asset
