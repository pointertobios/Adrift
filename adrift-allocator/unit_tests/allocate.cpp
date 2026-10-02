// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#include "adrift/allocator/allocate.h"
#include "adrift/allocator/types.h"

#include <array>
#include <cstring>
#include <format>
#include <print>

using namespace adrift;
using namespace adrift::allocator;

namespace {

usize g_failures{0};

#define CHECK(expr, ...)                                                                                   \
    do {                                                                                                   \
        if (!(expr)) {                                                                                     \
            ++g_failures;                                                                                  \
            std::println("[FAILED] {}:{}  {}  <{}>", __FILE__, __LINE__, std::format(__VA_ARGS__), #expr); \
        }                                                                                                  \
    } while (false)

void check_allocate_returns_non_null_pointer() {
    void *ptr = allocate(1, 1);
    CHECK(ptr != nullptr, "allocate(1, 1) 应返回非空指针");
    deallocate(ptr, 1, 1);
}

void check_allocate_respects_requested_alignment() {
    for (usize align : std::array<usize, 13>{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096}) {
        void *ptr = allocate(align, align);
        CHECK(ptr != nullptr, "对齐 {} 字节的分配应成功", align);
        CHECK(reinterpret_cast<usize>(ptr) % align == 0, "返回指针应满足 {} 字节对齐", align);

        deallocate(ptr, align, align);
    }
}

void check_slice_path_sizes_are_writable() {
    for (usize size : std::array<usize, 7>{1, 16, 17, 33, 64, 128, 256}) {
        auto *ptr = static_cast<u8 *>(allocate(size, 16));
        CHECK(ptr != nullptr, "分配 {} 字节应成功", size);

        std::memset(ptr, 0x3C, size);
        CHECK(ptr[0] == 0x3C && ptr[size - 1] == 0x3C, "分配 {} 字节的块应可读写", size);

        deallocate(ptr, size, 16);
    }
}

void check_overlapping_allocations_are_distinct() {
    constexpr usize block = 32;

    auto *first = static_cast<u8 *>(allocate(block, 16));
    auto *second = static_cast<u8 *>(allocate(block, 16));
    CHECK(first != nullptr && second != nullptr, "两次分配都应成功");
    CHECK(first != second, "两次未释放的分配应返回不同地址");

    std::memset(first, 0x11, block);
    std::memset(second, 0x22, block);

    CHECK(first[0] == 0x11 && first[block - 1] == 0x11, "第一块内容不应被第二块分配影响");
    CHECK(second[0] == 0x22 && second[block - 1] == 0x22, "第二块内容不应被第一块分配影响");

    deallocate(first, block, 16);
    deallocate(second, block, 16);
}

void check_consecutive_allocations_keep_their_contents() {
    constexpr usize count = 256;
    std::array<void *, count> blocks{};

    for (usize i = 0; i < count; ++i) {
        blocks[i] = allocate(sizeof(usize), alignof(usize));
        CHECK(blocks[i] != nullptr, "第 {} 次分配应成功", i);
        *static_cast<usize *>(blocks[i]) = i;
    }

    for (usize i = 0; i < count; ++i) {
        CHECK(*static_cast<usize *>(blocks[i]) == i, "第 {} 块的内容不应被其它分配覆盖", i);
    }

    for (void *ptr : blocks) {
        deallocate(ptr, sizeof(usize), alignof(usize));
    }
}

void check_released_block_is_reused_on_same_thread() {
    void *released = allocate(16, 16);
    CHECK(released != nullptr, "分配应成功");

    deallocate(released, 16, 16);

    void *reacquired = allocate(16, 16);
    CHECK(reacquired == released, "刚释放的块应被同线程的下一次分配复用");

    deallocate(reacquired, 16, 16);
}

void check_large_allocation_round_trip() {
    constexpr usize size = 64 * 1024;
    constexpr usize align = 4096;

    auto *ptr = static_cast<u8 *>(allocate(size, align));
    CHECK(ptr != nullptr, "大块分配应成功");
    CHECK(reinterpret_cast<usize>(ptr) % align == 0, "大块分配应满足 {} 字节对齐", align);

    ptr[0] = 0x5A;
    ptr[size - 1] = 0xA5;
    CHECK(ptr[0] == 0x5A && ptr[size - 1] == 0xA5, "大块分配应可读写整块内存");

    deallocate(ptr, size, align);
}

}  // namespace

int main() {
    check_allocate_returns_non_null_pointer();
    check_allocate_respects_requested_alignment();
    check_slice_path_sizes_are_writable();
    check_overlapping_allocations_are_distinct();
    check_consecutive_allocations_keep_their_contents();
    check_released_block_is_reused_on_same_thread();
    check_large_allocation_round_trip();

    if (g_failures != 0) {
        std::println("allocate：{} 项检查失败", g_failures);
        return 1;
    }
    std::println("allocate：全部检查通过");
    return 0;
}
