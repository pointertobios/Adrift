# Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
# SPDX-License-Identifier: MIT

include(${PROJECT_SOURCE_DIR}/cmake/GeneratePch.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/CollectFiles.cmake)

adrift_generate_pch(PRECOMPILE_HEADERS
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    EXCLUDE
        adrift/test
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch.h
)

adrift_generate_pch(PRECOMPILE_HEADERS_TEST
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/adrift/test
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch_test.h
)

adrift_collect_files(ADRIFT_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    PATTERNS "*.cpp"
    EXCLUDE
        test
)

add_library(adrift-tasks ${ADRIFT_SOURCES})
target_link_libraries(adrift-tasks adrift::base)
target_include_directories(adrift-tasks PUBLIC include)
target_precompile_headers(adrift-tasks PUBLIC ${PRECOMPILE_HEADERS})
add_library(adrift::tasks ALIAS adrift-tasks)

add_library(adrift-test src/test/test_main.cpp)
target_precompile_headers(adrift-test PUBLIC ${PRECOMPILE_HEADERS_TEST})
target_link_libraries(adrift-test PUBLIC adrift::tasks)
target_compile_definitions(adrift-test PUBLIC ADRIFT_TESTING)
add_library(adrift::test ALIAS adrift-test)

# Unit Tests

adrift_collect_files(ADRIFT_UNIT_TEST_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests
    PATTERNS "*.cpp"
)

add_executable(adrift-tasks-unit-tests ${ADRIFT_UNIT_TEST_SOURCES})
target_link_libraries(adrift-tasks-unit-tests PUBLIC adrift::test)
add_test(NAME adrift-tasks-unit-tests COMMAND adrift-tasks-unit-tests)
