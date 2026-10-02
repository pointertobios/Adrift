// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <optional>

#include "adrift/assert.h"
#include "adrift/constants.h"
#include "adrift/types/erased.h"
#include "adrift/types/int.h"
#include "adrift/types/ustr.h"

namespace adrift::os {

struct alignas(cacheline_size) shm_header {
    usize size;
    std::atomic<usize> holder_count;
    const u128 build_id;
};

static_assert(
    std::atomic<usize>::is_always_lock_free, "std::atomic<usize> 必须是无锁的才能用于共享内存进程间通信");

class shared_memory final {
public:
    /// 创建新的共享内存，若同名共享内存已存在则返回 std::nullopt
    static std::optional<shared_memory> create(const ustr &name, usize size);

    /// 持有一个已创建的共享内存，若不存在则返回 std::nullopt
    static std::optional<shared_memory> attach(const ustr &name);

    template<typename T, typename... Args>
    static std::optional<shared_memory> create(const ustr &name, Args &&...args) {
        auto shm = create(name, sizeof(T));
        if (shm.has_value()) {
            new (shm->get()) T(std::forward<Args>(args)...);
            shm->set_dtor(+[](void *ptr) { std::launder(reinterpret_cast<T *>(ptr))->~T(); });
        }
        return shm;
    }

    shared_memory(const shared_memory &) = delete;
    shared_memory &operator=(const shared_memory &) = delete;

    shared_memory(shared_memory &&) = default;
    shared_memory &operator=(shared_memory &&) = default;

    /// 持有者计数减一，归零时销毁共享内存
    ~shared_memory();

    /// 返回用户请求的共享内存大小（字节）
    usize size() const;

    /// 返回共享内存在本进程中的映射地址
    void *get() const;

    /// 设置析构回调，仅创建方（create 模式）可调用
    void set_dtor(void (*dtor)(void *)) {
        ADRIFT_ASSERT(m_is_create);

        m_dtor = dtor;
    }

private:
    shared_memory(bool is_create)
            : m_is_create{is_create} {}

    shared_memory with_extra(erased extra) && {
        m_extra = std::move(extra);
        return std::move(*this);
    }

    erased m_extra;
    void (*m_dtor)(void *) = nullptr;
    bool m_is_create;
};

};  // namespace adrift::os
