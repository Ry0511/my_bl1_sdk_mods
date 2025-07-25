//
// Date       : 21/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include <utility>

#include "pch.h"
#include "text_mod/text_mod.h"

#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"

namespace tm_parse {

TextMod::TextMod(const fs::path& pth) : m_SourceFile(pth) {
    if (!fs::exists(pth)) {
        throw std::runtime_error{std::format("file not found '{}'", pth.string())};
    }
    _read_file();
    _parse_text();
}

TextMod::TextMod(str mod_text) noexcept(false) : m_Text(std::move(mod_text)), m_SourceFile(std::nullopt) {
    _parse_text();
}

void TextMod::unload() {
    m_Program = ProgramRule{};

    // Can only do this if we can re-obtain the text
    if (m_SourceFile.has_value()) {
        m_Text = str{};
    }
}

void TextMod::reload() {
    if (m_SourceFile.has_value()) {
        _read_file();
    }
    _parse_text();
}

void tm_parse::TextMod::_parse_text() {
    TextModLexer lexer{m_Text};
    TextModParser parser{&lexer};
    m_Program = ProgramRule::create(parser);
}

void tm_parse::TextMod::_read_file() {
    str_fstream ss{*m_SourceFile};
    m_Text = str{str_istreambuf_it{ss}, str_istreambuf_it{}};
}

}  // namespace tm_parse