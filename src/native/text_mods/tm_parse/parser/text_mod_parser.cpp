//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "text_mod_parser.h"

namespace tm_parse {

constexpr size_t dflt_pool_size = 128;

TextModParser::TextModParser(TextModLexer* lexer) noexcept
    : m_RuleStack(),
      m_EndOfInputReached(false),
      m_Lexer(lexer),
      m_Index(0) {
    m_Tokens.reserve(dflt_pool_size);
}

SetCommandRule TextModParser::parse_set_cmd() {
    require_next<TokenKind::Kw_Set>();
    return SetCommandRule::create(*this);
}

ObjectIdentifierRule TextModParser::parse_object_def() {
    require_next<TokenKind::Kw_Begin>();
    return ObjectIdentifierRule::create(*this);
}

}  // namespace tm_parse