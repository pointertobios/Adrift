include(${PROJECT_SOURCE_DIR}/cmake/GeneratePch.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/CollectFiles.cmake)

jungle_generate_pch(PRECOMPILE_HEADERS
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pch.h
)

jungle_collect_files(REFLECTION-GEN_SOURCES
    INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    PATTERNS "*.cpp"
)

add_executable(reflection-gen ${REFLECTION-GEN_SOURCES})
target_link_libraries(reflection-gen PRIVATE jungle::infrastructure)
target_include_directories(reflection-gen PUBLIC include)
target_precompile_headers(reflection-gen PUBLIC ${PRECOMPILE_HEADERS})
