# Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 4.0)
project(Adrift VERSION 0.1.0 LANGUAGES CXX)

# Compiler and linker config

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(ProcessorCount)

ProcessorCount(CPUS)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Projects

add_subdirectory(adrift-infrastructure)
add_subdirectory(adrift-base)
add_subdirectory(adrift-allocator)
add_subdirectory(adrift-tasks)
add_subdirectory(adrift-core)
add_subdirectory(adrift-reflection)
add_subdirectory(tests)

# adftproj 3rd: cpptrace
add_subdirectory(${CMAKE_SOURCE_DIR}/.adftproj/3rd/cpptrace)
