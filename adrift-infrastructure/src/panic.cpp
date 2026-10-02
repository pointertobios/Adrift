// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/panic.h"

#include <cstdlib>
#include <print>

#include <cpptrace/cpptrace.hpp>

#include "adrift/os/raw_sync_io.h"

namespace adrift {

[[noreturn]] void panic(std::string_view msg, std::source_location sl) {
    std::string msg_final;
    if (!msg.empty()) {
        msg_final = std::format(": {}", msg);
    }
    std::println(stderr, "Panicked at {}:{}:{}{}", sl.file_name(), sl.line(), sl.column(), msg_final);
    std::println(stderr, "{}", cpptrace::generate_trace());
    std::abort();
}

[[noreturn]] void panic_oom() {
    os::write_stderr("Panicked because out of memory.");
    std::abort();
}

};  // namespace adrift
