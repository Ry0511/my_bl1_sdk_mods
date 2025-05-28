//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "text_mod_parser.h"

namespace tm_parse {

namespace {

ParserRuleKind get_rule_in_priority(TokenKind kind) {
    // Since we only have two primary nodes this is relatively simple. Parse as the primary nodes
    // and let the recursive descent parser rip the token stream apart.

    if (kind == TokenKind::Kw_Set) {
        return ParserRuleKind::SetCommand;
    }

    if (kind == TokenKind::Kw_Begin) {
        return ParserRuleKind::ObjectDefinition;
    }

    // Nothing actually should hit this unless the lexer encountered errors or the preivous parse
    // was incomplete in which case this is a parser error which needs to be handled.
    return ParserRuleKind::Unknown;
}

}  // namespace

void TextModParser::parse_internal(const Token& token) {

    if (token == TokenKind::Kw_Set) {
        push_token(token);
        SetCommandRule rule = SetCommandRule::create(this);
        TXT_LOG("[SET_COMMAND] ~ {}", rule.as_str(this));
    } else {
        TXT_LOG("Unknown token: '{}'", token.token_as_str());
    }
}

void TextModParser::parse_program() {

    // Default internal buffers
    m_Tokens.clear();
    m_Tokens.reserve(dflt_pool_size);

    m_Regions.clear();
    m_Regions.reserve(dflt_pool_size);

    Token token{};

    while (m_Lexer->read_token(&token)) {
        // Don't store these
        if (token.is_comment()) {
            continue;
        }

        // Store token
        parse_internal(token);
    }
}

}  // namespace tm_parse