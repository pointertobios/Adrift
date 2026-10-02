# Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
# SPDX-License-Identifier: MIT

include(${PROJECT_SOURCE_DIR}/cmake/GeneratePch.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/CollectFiles.cmake)

adrift_generate_pch(PRECOMPILE_HEADERS
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch.h
)

adrift_collect_files(ADRIFT_SOURCES_COMMON
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    PATTERNS "*.cpp"
    EXCLUDE
        test
        os/linux
        os/windows
)

if (LINUX)
    adrift_collect_files(ADRIFT_SOURCES_PLATFORM
        INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/os/linux
        PATTERNS "*.cpp"
    )
elseif (WIN32)
    adrift_collect_files(ADRIFT_SOURCES_PLATFORM
        INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/os/windows
        PATTERNS "*.cpp"
    )
endif()

set(ADRIFT_SOURCES ${ADRIFT_SOURCES_COMMON} ${ADRIFT_SOURCES_PLATFORM})

set(ADRIFT_DEPLIBS
    stdc++exp
)

if (LINUX)
    set(ADRIFT_DEPLIBS ${ADRIFT_DEPLIBS} ncurses)
elseif (WIN32)
    set(ADRIFT_DEPLIBS ${ADRIFT_DEPLIBS} atomic)
endif()

add_library(adrift-base ${ADRIFT_SOURCES})
target_precompile_headers(adrift-base PUBLIC ${PRECOMPILE_HEADERS})
target_link_libraries(adrift-base PUBLIC ${ADRIFT_DEPLIBS} adrift::infrastructure)
target_include_directories(adrift-base PUBLIC include)
add_library(adrift::base ALIAS adrift-base)
