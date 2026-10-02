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

if (WIN32)
    list(APPEND ADRIFT_DEPLIBS onecore)
endif()

set(ADRIFT_OPTIONS
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -freflection
    -fno-rtti
    -fno-exceptions
    -fconcepts-diagnostics-depth=6
)

include(AdriftBuildId)
adrift_generate_build_id(ADRIFT_BUILD_ID "${CMAKE_SOURCE_DIR}")

set(ADRIFT_DEFINES
    $<$<CONFIG:Debug>:ADRIFT_DEBUG_ENABLED>
    $<$<CONFIG:RelWithDebInfo>:ADRIFT_DEBUG_ENABLED>
    ADRIFT_BUILD_ID="${ADRIFT_BUILD_ID}"
)

set(ADRIFT_CONFIG_EMPTY_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/adrift-config-empty.cpp)
if(NOT EXISTS "${ADRIFT_CONFIG_EMPTY_SOURCE}")
    file(WRITE "${ADRIFT_CONFIG_EMPTY_SOURCE}" "")
endif()

add_library(adrift-config STATIC ${ADRIFT_CONFIG_EMPTY_SOURCE})
target_compile_options(adrift-config PUBLIC ${ADRIFT_OPTIONS})
target_compile_definitions(adrift-config PUBLIC ${ADRIFT_DEFINES})
add_library(adrift::config ALIAS adrift-config)

add_library(adrift-infrastructure ${ADRIFT_SOURCES})
target_link_libraries(adrift-infrastructure PRIVATE cpptrace::cpptrace)
target_precompile_headers(adrift-infrastructure PUBLIC ${PRECOMPILE_HEADERS})
target_link_libraries(adrift-infrastructure PUBLIC ${ADRIFT_DEPLIBS} adrift::config)
target_include_directories(adrift-infrastructure PUBLIC include)
add_library(adrift::infrastructure ALIAS adrift-infrastructure)
