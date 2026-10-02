// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/allocator/types.h"

#include <cstring>
#include <format>
#include <print>

using namespace jungle;
using namespace jungle::allocator;

namespace {

usize g_failures{0};

#define CHECK(expr, ...)                                                                                   \
    do {                                                                                                   \
        if (!(expr)) {                                                                                     \
            ++g_failures;                                                                                  \
            std::println("[FAILED] {}:{}  {}  <{}>", __FILE__, __LINE__, std::format(__VA_ARGS__), #expr); \
        }                                                                                                  \
    } while (false)

void check_bootstrap_yields_usable_arena() {
    arena *bootstrapped = arena::bootstrap();

    CHECK(bootstrapped != nullptr, "bootstrap 应返回一个可用的 arena");
    CHECK(bootstrapped->get_free_slices() > 0, "新建 arena 应至少有一个空闲 slice");
}

void check_slice_allocation_is_aligned_and_writable() {
    arena *target = arena::bootstrap();

    auto *slice = target->allocate_slice(0);
    CHECK(slice != nullptr, "arena 应能分配出一个 slice");
    CHECK(reinterpret_cast<usize>(slice) % arena::slice_size == 0, "分配的 slice 应按 slice 大小对齐");

    std::memset(slice, 0x77, arena::slice_size);
    CHECK(slice[0] == 0x77 && slice[arena::slice_size - 1] == 0x77, "分配的 slice 应可读写整块内存");

    target->deallocate_slice(slice, 0);
}

void check_slice_bookkeeping_is_balanced() {
    arena *target = arena::bootstrap();

    const auto before = target->get_free_slices();

    auto *slice = target->allocate_slice(0);
    CHECK(slice != nullptr, "arena 应能分配出一个 slice");
    CHECK(target->get_free_slices() == before - 1, "分配一个 slice 后空闲计数应减一");

    target->deallocate_slice(slice, 0);
    CHECK(target->get_free_slices() == before, "释放 slice 后空闲计数应恢复");
}

void check_multi_slice_allocation_reserves_whole_block() {
    arena *target = arena::bootstrap();
    constexpr usize size_class = 3;

    const auto before = target->get_free_slices();

    auto *slice = target->allocate_slice(size_class);
    CHECK(slice != nullptr, "arena 应能分配多 slice 的块");
    CHECK(
        target->get_free_slices() == before - units_of_size_class(size_class),
        "分配多 slice 块后空闲计数应按单元数减少");
    CHECK(reinterpret_cast<usize>(slice) % arena::slice_size == 0, "多 slice 块起始应仍按 slice 大小对齐");

    target->deallocate_slice(slice, size_class);
    CHECK(target->get_free_slices() == before, "释放多 slice 块后空闲计数应恢复");
}

}  // namespace

int main() {
    check_bootstrap_yields_usable_arena();
    check_slice_allocation_is_aligned_and_writable();
    check_slice_bookkeeping_is_balanced();
    check_multi_slice_allocation_reserves_whole_block();

    if (g_failures != 0) {
        std::println("arena：{} 项检查失败", g_failures);
        return 1;
    }
    std::println("arena：全部检查通过");
    return 0;
}
