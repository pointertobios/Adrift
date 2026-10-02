# Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
# SPDX-License-Identifier: MIT

include(${PROJECT_SOURCE_DIR}/cmake/GeneratePch.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/CollectFiles.cmake)

adrift_generate_pch(PRECOMPILE_HEADERS
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch.h
)

adrift_collect_files(ADRIFT_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    PATTERNS "*.cpp"
)

add_library(adrift-core ${ADRIFT_SOURCES})
target_link_libraries(adrift-core adrift::base adrift::tasks)
target_include_directories(adrift-core PUBLIC include)
target_precompile_headers(adrift-core PUBLIC ${PRECOMPILE_HEADERS})
add_library(adrift::core ALIAS adrift-core)

# Unit Tests

adrift_collect_files(ADRIFT_UNIT_TEST_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests
    PATTERNS "*.cpp"
)

add_executable(adrift-core-unit-tests ${ADRIFT_UNIT_TEST_SOURCES})
target_link_libraries(adrift-core-unit-tests PUBLIC adrift::test adrift::core)
add_test(NAME adrift-core-unit-tests COMMAND adrift-core-unit-tests)
