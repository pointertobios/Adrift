// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/runtime/daemon.h"
#include "adrift/os/process.h"

namespace adrift::runtime {

daemon::daemon()
        : m_name{"adft::daemon"} {
    m_construct_sem.release();
}

daemon::daemon(ustr name)
        : m_name{name} {
    m_construct_sem.release();
}

daemon::~daemon() { join(); }

void daemon::start() {
    m_thread = std::jthread{[this](std::stop_token st) { run(st); }};
}

void daemon::awake() { m_idle_sem.release(); }

void daemon::join() {
    if (m_joined) {
        return;
    }
    if (m_thread.joinable()) {
        m_thread.request_stop();
        awake();
        m_thread.join();
        m_joined = true;
    }
}

void daemon::wait_for_awake() { m_idle_sem.acquire(); }

void daemon::run(std::stop_token &st) {
    m_construct_sem.acquire();

    if (m_thread.joinable()) {
        os::thread_handle::from(m_thread).set_name(std::string{m_name.view()});
    } else {
        os::thread_handle::this_thread().set_name(std::string{m_name.view()});
    }
    if (!initialize()) {
        return;
    }
    while (run_once(st)) {}
    finalize();
}

};  // namespace adrift::runtime
