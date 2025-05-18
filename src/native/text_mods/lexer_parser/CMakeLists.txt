cmake_minimum_required(VERSION 3.23)

add_library(bl1_text_mods STATIC)
target_compile_features(bl1_text_mods PRIVATE cxx_std_20)
target_compile_definitions(bl1_text_mods PUBLIC -DTEXT_MODS_USE_WCHAR)

target_sources(
        bl1_text_mods
        PRIVATE
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

target_include_directories(bl1_text_mods PUBLIC ./)

add_executable(bl1_text_mods_exe Main.cpp)
target_compile_features(bl1_text_mods_exe PRIVATE cxx_std_20)
target_link_libraries(bl1_text_mods_exe PUBLIC bl1_text_mods)