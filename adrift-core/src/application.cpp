// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/core/application.h"

#include "adrift/assert.h"
#include "adrift/core/service/service.h"

namespace adrift::core {

Application &Application::current() {
    ADRIFT_ASSERT(s_application);
    return *s_application;
}

Application::Application(std::span<string_id> using_services) {
    ADRIFT_ASSERT(!s_application, "仅能构造一个 Application 实例");
    s_application = this;

    for (auto sid : using_services) {
        auto ctor = service::Service::get_service_creator(sid);
        auto [tid, uptr] = ctor();
        auto res = m_service_table.insert(tid, try_move(uptr));
        ADRIFT_ASSERT(res, "重复的服务 '{}'", uptr->name());
    }
}

async::future<> Application::run() {
    service::ServiceController ctrl;
    for (auto &service : m_service_table) {
        service.value()->start(ctrl);
    }
    for (auto &service : m_service_table) {
        co_await service.value()->join();
    }
}

};  // namespace adrift::core
