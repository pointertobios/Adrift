# Adrift 项目 AI 编程助手指引

Adrift 是一个实验性 C++26 游戏引擎，详情参见 [README.md](./README.md)。

## 编译器特性

本项目依赖 **GCC 且需要 C++26 扩展**（Clang/MSVC 不支持），关键标志：

- `-freflection` — P2996 编译期反射
- `-fno-rtti` / `-fno-exceptions` — 禁用 RTTI 与异常
- `-fconcepts-diagnostics-depth=6` — 概念（concept）诊断深度
- `-Wall -Wextra -Wpedantic -Werror` — 严格警告

**AI 编码注意**：不要使用 `try/catch`、`throw`、`dynamic_cast`、`typeid`。错误处理使用 `adrift::panic()` 或 `std::expected`。前置/后置条件断言统一使用 `ADRIFT_ASSERT`（仅 Debug 模式生效）。

## 模块架构

| 模块             | 命名空间         | 用途                                               | 状态   |
| ---------------- | ---------------- | -------------------------------------------------- | ------ |
| `adrift-infrastructure/` | `adrift::` | 无分配类型系统、工具、OS 进程/线程、构建 ID | ✅ 活跃 |
| `adrift-base/`   | `adrift::`       | 容器、代数、反射、序列化、调试、测试框架、字符串    | ✅ 活跃 |
| `adrift-core/`   | `adrift::core::` | ECS 核心（Entity、Component、Archetype、Manager）  | ✅ 活跃 |
| `adrift-api/`    | —                | 占位                                               | ❌      |
| `adrift-client/` | —                | 占位                                               | ❌      |
| `adrift-server/` | —                | 占位                                               | ❌      |
| `adrift-ui/`     | —                | 占位                                               | ❌      |

依赖：`adrift-core` → `adrift-base` → `adrift-infrastructure`；`adrift-tasks` → `adrift-base`（其他模块目前为空壳）。

## 命名与代码风格

- **文件头**：所有文件以 `// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>` 和 `// SPDX-License-Identifier: MIT` 开头
- **Include guard**：统一使用 `#pragma once`
- **命名空间**：模块 `adrift-X` 使用 `adrift::X::`，基础库使用 `adrift::`。闭合格式：`};  // namespace adrift::X`
- **类型名**：PascalCase（如 `ComponentID`、`hash_map`）
- **函数与变量**：snake_case
- **成员变量**：`m_` 前缀（如 `m_data`、`m_id`）
- **注释**：中文

## 测试

使用自定义测试框架，位于 `adrift-base/include/adrift/test/test.h`：

```cpp
#include "adrift/test/test.h"

ADRIFT_SYNC_TEST(test_name) {
    ADRIFT_SYNC_ASSERT(condition, "failure message {}", args...);
    ADRIFT_SYNC_SUCCESS();
}
```

- 单元测试：`<module>/unit_tests/*.cpp`
- 集成测试：`tests/`
- 测试库目标：`adrift::test`

## 关键自定义类型

- `adrift::ustr` — Unicode 字符串（核心类型，广泛使用）
- `adrift::hash_map<K,V>` — 自定义哈希表
- `adrift::panic()` — 致命错误（变参格式化版本可用）
- `adrift::debug(T&)` — 基于反射的调试打印
- 固定宽度整数类型定义于 `adrift/types/int.h`

## 文档

- 简体中文文档：[docs/zh-cn/src/](./docs/zh-cn/src/)
- 英文文档：[docs/en-us/src/](./docs/en-us/src/)
- 文档使用 mdBook 构建，参见 [SUMMARY.md](./docs/zh-cn/src/SUMMARY.md)
