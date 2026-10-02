// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/async/future.h"
#include "adrift/test/test.h"

using namespace adrift;

async::future<> async_func() { co_return; }

async::future<int> async_func_int() {
    co_await async_func();
    co_return 1;
}

ADRIFT_ASYNC_TEST(future_type_correctness) {
    co_await async_func_int();
    ADRIFT_ASYNC_SUCCESS();
}
