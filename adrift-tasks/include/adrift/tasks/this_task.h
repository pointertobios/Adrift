// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <coroutine>

#include "adrift/tasks/runtime/worker.h"
#include "adrift/async/invoke.h"

namespace adrift::tasks::this_task {

inline auto yield() {
    struct yield_awaitable {
        bool await_ready() const noexcept { return false; }

        void await_suspend(std::coroutine_handle<> handle) {
            auto &w = runtime::worker::current();
            w.set_next_resume(handle);
            w.set_yield_now();
        }

        void await_resume() const noexcept {}
    };
    return yield_awaitable{};
}

inline auto &worker_handle() {
    return runtime::worker::current().host_runtime().get_worker_handle_of_current_worker();
}

inline auto &worker() { return runtime::worker::current(); }

inline auto &host_runtime() { return runtime::worker::current().host_runtime(); }

inline auto id() { return runtime::worker::current().this_task_id(); }

template<typename... Args>
auto spawn(async::async_function<Args...> auto &&fn, Args &&...args) {
    return runtime::worker::current().host_runtime().spawn(
        std::forward<decltype(fn)>(fn), std::forward<Args>(args)...);
}

template<typename... Args>
auto spawn(runtime::worker_handle handle, async::async_function<Args...> auto &&fn, Args &&...args) {
    return runtime::worker::current().host_runtime().spawn(
        handle, std::forward<decltype(fn)>(fn), std::forward<Args>(args)...);
}

template<typename... Args>
auto spawn_blocking(std::invocable<Args...> auto &&fn, Args &&...args)
    requires(!async::async_function<decltype(fn), Args...>)
{
    return runtime::worker::current().host_runtime().spawn_blocking(
        std::forward<decltype(fn)>(fn), std::forward<Args>(args)...);
}

};  // namespace adrift::tasks::this_task

namespace adrift {

namespace this_task = tasks::this_task;

};  // namespace adrift
