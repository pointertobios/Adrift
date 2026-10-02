// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>

#include "adrift/async/future.h"
#include "adrift/sync/condition_variable.h"

namespace adrift::core::service {

class ServiceController {
public:
    ServiceController() = default;

    void stop() {
        m_stop.store(true, morder::release);
        m_cv.notify_all();
    }

    bool stop_requested() const { return m_stop.load(morder::acquire); }

    async::future<> wait_for_stop() const {
        co_await m_cv([&stop = this->m_stop] { return stop.load(morder::acquire); });
    }

private:
    std::atomic_bool m_stop{false};
    mutable sync::condition_variable m_cv{};
};

};  // namespace adrift::core::service
