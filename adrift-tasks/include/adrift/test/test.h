// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#ifdef ADRIFT_TESTING

#    include <expected>
#    include <source_location>
#    include <string>
#    include <string_view>
#    include <variant>

#    include "adrift/async/future.h"
#    include "adrift/preusing.h"

namespace adrift::test {

using test_result = std::expected<void, ustr>;
using sync_test_function = test_result (*)();
using async_test_function = async::future<test_result> (*)();
using test_function = std::variant<sync_test_function, async_test_function>;

template<std::same_as<bool>... Args>
consteval bool ignore_state(Args &&...args) {
    return (args || ...);
}

bool add_test(std::string_view filename, std::string name, test_function fn, bool ignore);

#    define ADRIFT_IGNORE_TEST true

#    define ADRIFT_SYNC_TEST(name, ...)                                                              \
        static adrift::test::test_result test_##name();                                               \
        [[maybe_unused]] static bool test_##name##_registered = adrift::test::add_test(               \
            __FILE__, #name, test_##name, adrift::test::ignore_state(__VA_ARGS__));                    \
        static adrift::test::test_result test_##name()

#    define ADRIFT_SYNC_ASSERT(expr, ...)                                                             \
        do {                                                                                          \
            if (!(expr)) {                                                                            \
                auto location = std::source_location::current();                                      \
                return std::unexpected{adrift::ustr::format(                                          \
                    "  at {}:{}\n{} evaluated false:  {}", location.file_name(), location.line(), #expr, \
                    adrift::ustr::format(__VA_ARGS__))};                                              \
            }                                                                                         \
        } while (false)

#    define ADRIFT_SYNC_SUCCESS() return adrift::test::test_result{}

#    define ADRIFT_ASYNC_TEST(name, ...)                                                             \
        static adrift::async::future<adrift::test::test_result> test_##name();                        \
        [[maybe_unused]] static bool test_##name##_registered = adrift::test::add_test(               \
            __FILE__, #name, test_##name, adrift::test::ignore_state(__VA_ARGS__));                   \
        static adrift::async::future<adrift::test::test_result> test_##name()

#    define ADRIFT_ASYNC_ASSERT(expr, ...)                                                            \
        do {                                                                                          \
            if (!(expr)) {                                                                            \
                auto location = std::source_location::current();                                      \
                co_return std::unexpected{adrift::ustr::format(                                       \
                    "  at {}:{}\n{} evaluated false:  {}", location.file_name(), location.line(), #expr, \
                    adrift::ustr::format(__VA_ARGS__))};                                              \
            }                                                                                         \
        } while (false)

#    define ADRIFT_ASYNC_SUCCESS() co_return adrift::test::test_result{}

};  // namespace adrift::test

#endif
