// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>
#include <optional>
#include <type_traits>

namespace adrift {

class ustr;

};

namespace adrift::concepts {

template<typename T>
concept Debug = requires(T t) {
    { t.debug() } -> std::same_as<ustr>;
};

template<typename T>
concept is_void = std::is_void_v<T>;

template<typename T>
concept non_void = !std::is_void_v<T>;

template<typename T>
concept is_enum = std::is_enum_v<T>;

template<typename Fn, typename Ret, typename... Args>
concept verified_invocable = std::is_invocable_r_v<Ret, Fn, Args...>;

template<typename T>
concept deref_nullable = std::convertible_to<std::remove_cvref_t<T>, bool> && requires(T value) {
    { *value };
};

};  // namespace adrift::concepts
