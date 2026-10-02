// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <coroutine>
#include <deque>
#include <functional>
#include <optional>
#include <queue>

#include "adrift/container/hash_map.h"
#include "adrift/container/mpsc.h"
#include "adrift/sync/spinlock.h"
#include "adrift/types/raw_storage.h"

namespace adrift::tasks::runtime {

using task_id = void *;

struct task {
    task_id m_id{};
    std::coroutine_handle<> m_resume_handle{};

    operator bool() const { return m_id != nullptr; }
};

};  // namespace adrift::tasks::runtime

namespace adrift::tasks::runtime::sched {

class scheduler final {
    static constexpr usize reschedule_slots_size = 16;

    struct reschedule_slot {
        raw_storage<task> m_task{};
        bool available{false};
    };

public:
    void attach_task(const task &t);
    void detach_task(task_id tid);
    std::optional<task> next_task();
    void resched(task t);
    void suspend(task t);
    void awake(task_id id);

    bool has_suspended() const;

    task steal();

private:
    void awake_impl(task_id id);

    task_id m_current_task;

    std::array<reschedule_slot, reschedule_slots_size> m_reschedule_slots;
    sync::spinlock<std::deque<task>> m_queue;
    hash_map<task_id, task> m_suspended_tasks;
    hash_set<task_id> m_preawake;

    std::tuple<container::mpsc<task_id>::sender, container::mpsc<task_id>::receiver> m_pending_awake{
        container::mpsc<task_id>::queue()};
};

};  // namespace adrift::tasks::runtime::sched