# Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
# SPDX-License-Identifier: MIT

include(${PROJECT_SOURCE_DIR}/cmake/GeneratePch.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/CollectFiles.cmake)

jungle_generate_pch(PRECOMPILE_HEADERS
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    EXCLUDE
        jungle/allocator/extension
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch.h
)

jungle_generate_pch(PRECOMPILE_HEADERS_EXT
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/jungle/allocator/extension
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch_ext.h
)

jungle_collect_files(JUNGLE_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    PATTERNS "*.cpp"
    EXCLUDE
        extension
)

add_library(jungle-allocator SHARED ${JUNGLE_SOURCES})
target_include_directories(jungle-allocator PUBLIC include)
target_link_libraries(jungle-allocator PUBLIC jungle::config jungle::infrastructure)
target_precompile_headers(jungle-allocator PUBLIC ${PRECOMPILE_HEADERS})
add_library(jungle::allocator ALIAS jungle-allocator)

jungle_collect_files(JUNGLE_EXT_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/extension
    PATTERNS "*.cpp"
)

add_library(jungle-allocator-ext STATIC ${JUNGLE_EXT_SOURCES})
target_link_libraries(jungle-allocator-ext PRIVATE jungle::allocator)
target_precompile_headers(jungle-allocator-ext PUBLIC ${PRECOMPILE_HEADERS_EXT})
add_library(jungle::allocator-ext ALIAS jungle-allocator-ext)

# Unit Tests

add_executable(jungle-allocator-size-class-test unit_tests/size_class.cpp)
target_link_libraries(jungle-allocator-size-class-test PRIVATE jungle::allocator)
add_test(NAME jungle-allocator-size-class-test COMMAND jungle-allocator-size-class-test)

add_executable(jungle-allocator-arena-test unit_tests/arena.cpp)
target_link_libraries(jungle-allocator-arena-test PRIVATE jungle::allocator)
add_test(NAME jungle-allocator-arena-test COMMAND jungle-allocator-arena-test)

add_executable(jungle-allocator-allocate-test unit_tests/allocate.cpp)
target_link_libraries(jungle-allocator-allocate-test PRIVATE jungle::allocator)
add_test(NAME jungle-allocator-allocate-test COMMAND jungle-allocator-allocate-test)
