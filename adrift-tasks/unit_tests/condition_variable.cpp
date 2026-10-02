// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/sync/condition_variable.h"
#include "adrift/tasks/this_task.h"
#include "adrift/test/test.h"

using namespace adrift;
using namespace adrift::sync;

ADRIFT_SYNC_TEST(condition_variable_default_construction_and_destruction) {
    condition_variable cv;
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(notify_one_with_no_waiters_returns_false) {
    condition_variable cv;

    auto result = cv.notify_one();
    ADRIFT_SYNC_ASSERT(!result, "无等待者时 notify_one 应返回 false");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_SYNC_TEST(notify_all_with_no_waiters_returns_zero) {
    condition_variable cv;

    auto count = cv.notify_all();
    ADRIFT_SYNC_ASSERT(count == 0, "无等待者时 notify_all 应返回 0");
    ADRIFT_SYNC_SUCCESS();
}

ADRIFT_ASYNC_TEST(wait_and_notify_one_resumes_waiter) {
    condition_variable cv;
    bool ready = false;
    bool woken = false;

    auto jh = this_task::spawn([&]() -> async::future<> {
        co_await cv([&] { return ready; });
        woken = true;
    });

    ready = true;
    cv.notify_one();

    co_await jh;
    ADRIFT_ASYNC_ASSERT(woken, "notify_one 后等待者应被唤醒");
    ADRIFT_ASYNC_SUCCESS();
}

ADRIFT_ASYNC_TEST(predicate_wait_resumes_when_condition_becomes_true) {
    condition_variable cv;
    bool ready = false;
    bool done = false;

    auto jh = this_task::spawn([&]() -> async::future<> {
        co_await cv([&] { return ready; });
        done = true;
    });

    ready = true;
    cv.notify_one();

    co_await jh;
    ADRIFT_ASYNC_ASSERT(done, "谓词成立后等待者应恢复执行");
    ADRIFT_ASYNC_SUCCESS();
}

ADRIFT_ASYNC_TEST(notify_all_wakes_multiple_waiters) {
    condition_variable cv;
    bool ready = false;
    int woken_count = 0;

    auto waiter = [&]() -> async::future<> {
        co_await cv([&] { return ready; });
        woken_count++;
    };

    auto jh1 = this_task::spawn(waiter);
    auto jh2 = this_task::spawn(waiter);
    auto jh3 = this_task::spawn(waiter);

    ready = true;
    cv.notify_all();

    co_await jh1;
    co_await jh2;
    co_await jh3;

    ADRIFT_ASYNC_ASSERT(woken_count == 3, "所有等待者应被唤醒");
    ADRIFT_ASYNC_SUCCESS();
}
