# Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
# SPDX-License-Identifier: MIT

include(${PROJECT_SOURCE_DIR}/cmake/GeneratePch.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/CollectFiles.cmake)

adrift_generate_pch(PRECOMPILE_HEADERS
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    EXCLUDE
        adrift/allocator/extension
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch.h
)

adrift_generate_pch(PRECOMPILE_HEADERS_EXT
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/adrift/allocator/extension
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch_ext.h
)

adrift_collect_files(ADRIFT_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    PATTERNS "*.cpp"
    EXCLUDE
        extension
)

add_library(adrift-allocator SHARED ${ADRIFT_SOURCES})
target_include_directories(adrift-allocator PUBLIC include)
target_link_libraries(adrift-allocator PUBLIC adrift::config adrift::infrastructure)
target_precompile_headers(adrift-allocator PUBLIC ${PRECOMPILE_HEADERS})
add_library(adrift::allocator ALIAS adrift-allocator)

adrift_collect_files(ADRIFT_EXT_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/extension
    PATTERNS "*.cpp"
)

add_library(adrift-allocator-ext STATIC ${ADRIFT_EXT_SOURCES})
target_link_libraries(adrift-allocator-ext PRIVATE adrift::allocator)
target_precompile_headers(adrift-allocator-ext PUBLIC ${PRECOMPILE_HEADERS_EXT})
add_library(adrift::allocator-ext ALIAS adrift-allocator-ext)

# Unit Tests

add_executable(adrift-allocator-size-class-test unit_tests/size_class.cpp)
target_link_libraries(adrift-allocator-size-class-test PRIVATE adrift::allocator)
add_test(NAME adrift-allocator-size-class-test COMMAND adrift-allocator-size-class-test)

add_executable(adrift-allocator-arena-test unit_tests/arena.cpp)
target_link_libraries(adrift-allocator-arena-test PRIVATE adrift::allocator)
add_test(NAME adrift-allocator-arena-test COMMAND adrift-allocator-arena-test)

add_executable(adrift-allocator-allocate-test unit_tests/allocate.cpp)
target_link_libraries(adrift-allocator-allocate-test PRIVATE adrift::allocator)
add_test(NAME adrift-allocator-allocate-test COMMAND adrift-allocator-allocate-test)
