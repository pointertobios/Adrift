// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "jungle/allocator/types.h"

#include <array>
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

struct alignas(16) sixteen_byte_object {
    u8 bytes[16];
};

struct alignas(64) sixty_four_byte_object {
    u8 bytes[64];
};

void check_lowest_unit() { CHECK(lowest_unit == 16, "最小分配单元应为 16 字节"); }

void check_arena_geometry() {
    CHECK(arena::slice_size == 0x1'000, "单个 slice 应为 4096 字节");
    CHECK(arena::size_classes == 1024, "arena 应划分出 1024 个 size class");
    CHECK(arena::size_classes * arena::slice_size == 0x400'000, "arena 容量应为 4 MiB");
}

void check_size_class_boundaries() {
    CHECK(size_class_of(1) == 0, "1 字节应归入 0 号 size class");
    CHECK(size_class_of(16) == 0, "16 字节应归入 0 号 size class");
    CHECK(size_class_of(17) == 1, "17 字节应归入 1 号 size class");
    CHECK(size_class_of(32) == 1, "32 字节应归入 1 号 size class");
    CHECK(size_class_of(33) == 2, "33 字节应归入 2 号 size class");
}

void check_slice_boundary() {
    CHECK(
        size_class_of(16 * arena::size_classes) == arena::size_classes - 1,
        "16 KiB 应归入 slice 路径内最大的 size class");
    CHECK(
        size_class_of(16 * arena::size_classes + 1) == arena::size_classes,
        "超过 16 KiB 应离开 slice 分配路径");
}

void check_unit_counts_round_trip() {
    for (usize size : std::array<usize, 6>{1, 16, 17, 64, 1023, 4096}) {
        const auto size_class = size_class_of(size);
        CHECK(units_of(size) == units_of_size_class(size_class), "size 与 size class 换算出的单元数应一致");
        CHECK(size_class_of_units(units_of(size)) == size_class, "单元数应能反推回原 size class");
    }
}

void check_unit_size_covers_request() {
    for (usize size : std::array<usize, 6>{1, 15, 16, 17, 1000, 16384}) {
        const auto bytes = units_of(size) * lowest_unit;
        CHECK(bytes >= size, "单元大小应不小于请求大小");
        CHECK(bytes - lowest_unit < size, "单元大小应是覆盖请求的最小 16 字节倍数");
    }
}

void check_type_traits() {
    CHECK(size_class_of_v<u8> == 0 && units_of_v<u8> == 1, "1 字节类型应占 1 个 0 号单元");
    CHECK(size_class_of_v<u64> == 0 && units_of_v<u64> == 1, "8 字节类型应占 1 个 0 号单元");
    CHECK(
        size_class_of_v<sixteen_byte_object> == 0 && units_of_v<sixteen_byte_object> == 1,
        "16 字节类型应占 1 个单元");
    CHECK(
        size_class_of_v<sixty_four_byte_object> == 3 && units_of_v<sixty_four_byte_object> == 4,
        "64 字节类型应占 4 个单元");
}

void check_slice_descriptor_layout() {
    CHECK(sizeof(slice_descriptor) == 64, "slice_descriptor 应为 64 字节");
    CHECK(
        sizeof(slice_descriptor_slice::m_storage) == 63 * sizeof(slice_descriptor),
        "描述符切片应容纳 63 个内嵌 slice_descriptor 槽位");
    CHECK(sizeof(slice_descriptor_slice) == arena::slice_size, "slice_descriptor_slice 应恰好占满一个 slice");
}

}  // namespace

int main() {
    check_lowest_unit();
    check_arena_geometry();
    check_size_class_boundaries();
    check_slice_boundary();
    check_unit_counts_round_trip();
    check_unit_size_covers_request();
    check_type_traits();
    check_slice_descriptor_layout();

    if (g_failures != 0) {
        std::println("size_class：{} 项检查失败", g_failures);
        return 1;
    }
    std::println("size_class：全部检查通过");
    return 0;
}
