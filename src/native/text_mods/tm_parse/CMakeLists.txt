cmake_minimum_required(VERSION 3.23)

################################################################################
# | LEXER & PARSER TARGET |
################################################################################

add_library(tm_parse_ INTERFACE)
target_precompile_headers(tm_parse_ INTERFACE ./pch.h)
target_compile_features(tm_parse_ INTERFACE cxx_std_20)

if (MSVC)
    target_compile_options(tm_parse_ INTERFACE "/Zc:preprocessor")
endif ()

# Glob the fat fucking smelly sources
file(GLOB_RECURSE TM_PARSE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
list(FILTER TM_PARSE_SRC EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/standalone/")

target_sources(tm_parse_ INTERFACE ${TM_PARSE_SRC})

target_include_directories(tm_parse_ INTERFACE ./ ../)

# External module usage with unrealsdk
add_library(tm_parse INTERFACE)
target_link_libraries(tm_parse INTERFACE tm_parse_)
target_compile_definitions(tm_parse INTERFACE -DTEXT_MODS_UNREALSDK -DTEXT_MODS_USE_WCHAR)
target_link_libraries(tm_parse INTERFACE unrealsdk)

# Standalone module
add_library(tm_parse_solo INTERFACE)
target_link_libraries(tm_parse_solo INTERFACE tm_parse_)
target_compile_definitions(tm_parse_solo INTERFACE -DTEXT_MODS_STANDALONE)

################################################################################
# | STANDALONE BUILD TARGET |
################################################################################

add_executable(bl1_text_mods_exe EXCLUDE_FROM_ALL)

file(
        GLOB_RECURSE
        TM_PARSE_STANDALONE_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/standalone/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/standalone/tests/*.h"
)
target_sources(bl1_text_mods_exe PRIVATE ${TM_PARSE_STANDALONE_SRC} standalone/main.cpp)

target_compile_features(bl1_text_mods_exe PRIVATE cxx_std_20)
target_link_libraries(bl1_text_mods_exe PRIVATE tm_parse_solo)
target_include_directories(bl1_text_mods_exe PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/standalone")

add_subdirectory(standalone/treevis)
