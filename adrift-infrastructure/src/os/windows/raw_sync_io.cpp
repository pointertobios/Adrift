// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/os/raw_sync_io.h"

#ifndef NOMINMAX
#    define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace adrift::os {

namespace {

void write_handle(::DWORD handle_id, std::string_view str) {
    const auto handle = ::GetStdHandle(handle_id);
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
        return;
    }

    ::DWORD written = 0;
    ::WriteFile(handle, str.data(), static_cast<::DWORD>(str.size()), &written, nullptr);
}

}  // namespace

void write_stdout(std::string_view str) { write_handle(STD_OUTPUT_HANDLE, str); }

void write_stderr(std::string_view str) { write_handle(STD_ERROR_HANDLE, str); }

};  // namespace adrift::os
