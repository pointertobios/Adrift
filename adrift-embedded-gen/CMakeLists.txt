# Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
# SPDX-License-Identifier: MIT

include(${PROJECT_SOURCE_DIR}/cmake/GeneratePch.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/CollectFiles.cmake)

adrift_generate_pch(PRECOMPILE_HEADERS
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch.h
)

adrift_collect_files(EMBEDDED-JAML-GEN_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    PATTERNS "*.cpp"
)

add_executable(adrift-embedded-jaml-gen ${EMBEDDED-JAML-GEN_SOURCES})
target_link_libraries(adrift-embedded-jaml-gen PRIVATE adrift::core)
target_link_libraries(adrift-embedded-jaml-gen PRIVATE adrift::config)
target_include_directories(adrift-embedded-jaml-gen PUBLIC include)
target_precompile_headers(adrift-embedded-jaml-gen PUBLIC ${PRECOMPILE_HEADERS})
