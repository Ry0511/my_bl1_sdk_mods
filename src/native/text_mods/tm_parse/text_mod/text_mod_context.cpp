//
// Date       : 20/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "text_mod/text_mod_context.h"
#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"

namespace tm_parse {

void TextModContext::load_from_file(const fs::path& pth) {
    m_LocalStr->clear();  // Always Invalidate

    try {
        str_fstream ss{pth};
        m_LocalStr = std::make_unique<str>(str_istreambuf_it{ss}, str_istreambuf_it{});
        load_from_str(*m_LocalStr);

    } catch (const std::exception& ex) {
        m_Program = ProgramRule{};
        TXT_LOG("Error parsing file: {}", ex.what());
    }
}

void TextModContext::load_from_str(str_view vw) {
    try {
        TextModLexer lexer{vw};
        TextModParser parser{&lexer};
        m_Program = ProgramRule::create(parser);
    } catch (const std::exception& ex) {
        m_Program = ProgramRule{};
        TXT_LOG("Error parsing file: {}", ex.what());
    }
}

}  // namespace tm_parse