// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <random>
#include <semaphore>
#include <stop_token>
#include <thread>
#include <type_traits>

#include "adrift/assert.h"
#include "adrift/async/future.h"
#include "adrift/async/invoke.h"
#include "adrift/async/join_handle.h"
#include "adrift/container/hash_map.h"
#include "adrift/container/mpsc.h"
#include "adrift/preusing.h"
#include "adrift/tasks/runtime/blocking_worker.h"
#include "adrift/tasks/runtime/debug_host.h"
#include "adrift/tasks/runtime/worker.h"
#include "adrift/types/concepts.h"
#include "adrift/util/rng.h"

namespace adrift::tasks::runtime {

class runtime;

class runtime_config {
    friend class runtime;

public:
    runtime_config() = default;

    runtime build() &&;
    std::unique_ptr<runtime> build_ptr() &&;

    runtime_config single_threaded() &&;
    runtime_config multi_threaded(usize n = std::thread::hardware_concurrency()) &&;

private:
    bool m_multi_thread{true};
    usize m_concurrency{std::thread::hardware_concurrency()};
};

class runtime final {
    friend class runtime_config;

public:
    explicit runtime();
    explicit runtime(runtime_config config);
    ~runtime();

    usize worker_count() const { return m_workers.size(); }
    worker &get_worker(usize wid) { return *m_workers[wid]; }

#ifdef ADRIFT_DEBUG_ENABLED
    debug_host &get_debug_host() { return *m_debug_host; }
#endif

private:
    bool spawn_impl(usize target_worker_id, task_item ta);

    usize get_most_available_worker_id();
    usize get_or_create_id_map_of_worker_handle(worker_handle &handle);

public:
    worker_handle &get_worker_handle_of_current_worker();

    template<typename... Args>
    auto spawn(async::async_function<Args...> auto &&fn, Args &&...args) {
        auto jh =
            task_coroutine(async::co_invoke(std::forward<decltype(fn)>(fn), std::forward<Args>(args)...));

        auto ta = jh.get_task_item();

        auto x = get_most_available_worker_id();
        while (!spawn_impl(x, ta)) {
            x = (x + 1) % m_blocking_pool_start;
        }

        return jh;
    }

    template<typename... Args>
    auto spawn(worker_handle target_handle, async::async_function<Args...> auto &&fn, Args &&...args) {
        auto worker_id = get_or_create_id_map_of_worker_handle(target_handle);

        auto jh =
            task_coroutine(async::co_invoke(std::forward<decltype(fn)>(fn), std::forward<Args>(args)...));

        auto ta = jh.get_task_item();

        while (!spawn_impl(worker_id, ta)) {
            std::this_thread::yield();
        }

        return jh;
    }

    template<typename... Args>
    auto block_on(async::async_function<Args...> auto &&fn, Args &&...args) {
        ADRIFT_ASSERT(m_multi_threaded);

        auto jh = spawn(std::forward<decltype(fn)>(fn), std::forward<Args>(args)...);

        if constexpr (concepts::is_void<typename decltype(jh)::output_type>) {
            jh.blocking_await();
        } else {
            return jh.blocking_await();
        }
    }

    template<typename... Args>
    auto spawn_blocking(std::invocable<Args...> auto &&fn, Args &&...args)
        requires(!async::async_function<decltype(fn), Args...>) {
        ADRIFT_ASSERT(m_multi_threaded);

        auto jh = blocking_task_coroutine(std::forward<decltype(fn)>(fn), std::forward<Args>(args)...);

        usize x;
        while (true) {
            auto wid = m_acceptible_blocking_worker_rx.lock()->recv();
            if (wid.has_value()) {
                x = wid.value();
                break;
            } else if (wid.error() == container::receive_failed::empty) {
                auto [tx, rx] = container::mpsc<task_item>::queue();
                auto atx = m_acceptible_blocking_worker_tx;

                auto g = m_blocking_workers.write();
                x = m_worker_id_gen++;
                auto &info = g->emplace_back(
                    std::make_unique<blocking_worker>(this, x, std::move(rx), std::move(atx)), std::move(tx));
                auto w = info.m_worker.get();
                { auto _ = std::move(g); }

                w->start();
            }
        }
        x -= m_blocking_pool_start;

        {
            auto g = m_blocking_workers.read();
            auto &info = g->at(x);
            (void)info.m_sender.send(jh.get_task_item());
            info.m_worker->awake();
        }

        return jh;
    }

    void main_loop() const;

    void stop() const;

private:
    auto task_coroutine(async::future_type auto future_value)
        -> async::join_handle<typename decltype(future_value)::output_type> {
        if (!m_multi_threaded) {
            stop();
        }
        using output_type = typename decltype(future_value)::output_type;
        if constexpr (concepts::is_void<output_type>) {
            co_await future_value;
            co_return;
        } else {
            co_return co_await future_value;
        }
    }

    template<typename... Args>
    auto blocking_task_coroutine(std::invocable<Args...> auto fn, Args &&...args)
        -> async::join_handle<std::invoke_result_t<decltype(fn)>> {
        if constexpr (concepts::is_void<std::invoke_result_t<decltype(fn)>>) {
            std::invoke(fn, args...);
            co_return;
        } else {
            co_return std::invoke(fn, args...);
        }
    }

    const bool m_multi_threaded;
    const usize m_blocking_pool_start;

    std::stop_source m_stop;

    usize m_worker_id_gen;

    std::vector<std::unique_ptr<worker>> m_workers{};
    std::vector<task_sender> m_senders{};
    sync::spinlock<container::mpsc<usize>::receiver> m_acceptible_worker_rx;

    struct blocking_worker_info {
        std::unique_ptr<blocking_worker> m_worker;
        task_sender m_sender;
    };

    sync::rwspinlock<std::vector<blocking_worker_info>, true> m_blocking_workers{};
    sync::spinlock<container::mpsc<usize>::receiver> m_acceptible_blocking_worker_rx;
    container::mpsc<usize>::sender m_acceptible_blocking_worker_tx;

    sync::rwspinlock<hash_map<usize, usize>> m_worker_handle_id_map{};

#ifdef ADRIFT_DEBUG_ENABLED
    std::unique_ptr<debug_host> m_debug_host{std::make_unique<debug_host>()};
#endif
};

};  // namespace adrift::tasks::runtime
