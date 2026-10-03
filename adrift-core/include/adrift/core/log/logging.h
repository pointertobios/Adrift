// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "adrift/core/service/service.h"

namespace adrift::core::log {

class Logging : public service::Service {
public:
    Logging();
    ~Logging() override = default;

    ustr name() const override { return "Logging"; }

private:
    async::future<> run(service::ServiceController &service_ctrl) override;
};

adrift_core_service_register(Logging);

};  // namespace adrift::core::log
