//
// Date       : 02/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "primitive_expr_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

using namespace txt;

NumberExprRule NumberExprRule::create(TextModParser& parser) {
    const Token& token = parser.peek();
    TXT_MOD_ASSERT(token == TokenKind::Number, "logic error");

    NumberExprRule rule{};
    rule.m_TextRegion = token.TextRegion;

    str text = str{rule.to_string(parser)};
    try {
        if (text.find_first_of(lit::dot) != str_view::npos) {
            rule.m_Value = std::stof(text);
        } else {
            rule.m_Value = std::stoi(text);
        }
    } catch (const std::exception& err) {
        throw std::runtime_error(std::format("Failed to convert '{}' with reason '{}'", text, err.what()));
    }

    return rule;
}

BoolExprRule BoolExprRule::create(TextModParser& parser) {
    const Token& token = parser.peek();
    TXT_MOD_ASSERT((token.is_any<TokenKind::Kw_True, TokenKind::Kw_False>()), "logic error");

    BoolExprRule rule{};
    rule.m_TextRegion = token.TextRegion;
    rule.m_Value = (token == TokenKind::Kw_True);
    return rule;
}

NoneExprRule NoneExprRule::create(TextModParser& parser) {
    const Token& token = parser.peek();
    TXT_MOD_ASSERT((token == TokenKind::Kw_None), "logic error");

    NoneExprRule rule{};
    rule.m_TextRegion = token.TextRegion;
    return rule;
}

StrExprRule StrExprRule::create(TextModParser& parser) {
    const Token& token = parser.peek();
    TXT_MOD_ASSERT((token == TokenKind::StringLiteral), "logic error");
    StrExprRule rule{};
    rule.m_TextRegion = token.TextRegion;
    return rule;
}

NameExprRule NameExprRule::create(TextModParser& parser) {
    const Token& token = parser.peek();
    TXT_MOD_ASSERT((token == TokenKind::Identifier), "logic error");

    NameExprRule rule{};
    rule.m_TextRegion = token.TextRegion;

    parser.require_next<TokenKind::NameLiteral>();
    rule.m_TextRegion.extend(parser.peek().TextRegion);

    return rule;
}

PrimitiveExprRule PrimitiveExprRule::create(TextModParser&) {
    return PrimitiveExprRule{};
}

}  // namespace tm_parse::rules