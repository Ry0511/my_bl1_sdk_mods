cmake_minimum_required(VERSION 3.23)

################################################################################
# | LEXER & PARSER TARGET |
################################################################################

add_library(tm_parse_ INTERFACE)
target_precompile_headers(tm_parse_ INTERFACE ./pch.h)
target_compile_features(tm_parse_ INTERFACE cxx_std_20)

target_sources(
        tm_parse_
        INTERFACE
        text_mod_common.h
        text_mod_tokens.h
        text_mod_lexer.h
        text_mod_lexer.cpp
        text_mod_parser.h
        text_mod_parser.cpp
        text_mod_utils.h
        text_mod_utils.cpp
        text_mod_errors.h
)

target_include_directories(tm_parse_ INTERFACE ./)

# External module usage with unrealsdk
add_library(tm_parse INTERFACE)
target_link_libraries(tm_parse INTERFACE tm_parse_)
target_compile_definitions(tm_parse INTERFACE -DTEXT_MODS_UNREALSDK -DTEXT_MODS_USE_WCHAR)
target_link_libraries(tm_parse INTERFACE unrealsdk)

# Standalone module
add_library(tm_parse_solo INTERFACE)
target_link_libraries(tm_parse_solo INTERFACE tm_parse_)
target_compile_definitions(tm_parse_solo INTERFACE -DTEXT_MODS_STANDALONE -DTEXT_MODS_USE_WCHAR)

################################################################################
# | STANDALONE BUILD TARGET |
################################################################################

add_executable(bl1_text_mods_exe Main.cpp)
target_compile_features(bl1_text_mods_exe PRIVATE cxx_std_20)
target_link_libraries(bl1_text_mods_exe PRIVATE tm_parse_solo)