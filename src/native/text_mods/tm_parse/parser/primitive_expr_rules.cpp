//
// Date       : 02/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "primitive_expr_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

using namespace txt;
using namespace tokens;

NumberExprRule NumberExprRule::create(TextModParser& parser) {

    parser.require<Number>();
    const Token& token = parser.peek(-1);

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

StrExprRule StrExprRule::create(TextModParser& parser) {
    parser.require<StringLiteral>();
    StrExprRule rule{};
    const Token& token = parser.peek(-1);
    rule.m_TextRegion = token.TextRegion;

    return rule;
}

NameExprRule NameExprRule::create(TextModParser& parser) {

    parser.require<Identifier>();
    const Token& token = parser.peek(-1);

    NameExprRule rule{};
    rule.m_TextRegion = token.TextRegion;

    parser.require<TokenKind::NameLiteral>();
    auto literal_region = parser.peek(-1).TextRegion;
    rule.m_TextRegion.extend(literal_region);

    // Hack because I am lazy and don't want to mess with the lexer; -2 for both single quotes
    auto text = parser.text().substr(literal_region.Start+1, literal_region.Length-2);
    TextModLexer temp_lexer{text};
    TextModParser temp_parser{&temp_lexer};
    rule.m_Identifier = ObjectIdentifierRule::create(temp_parser);

    return rule;
}

KeywordRule KeywordRule::create(TextModParser& parser) {

    parser.require<Identifier>();
    const Token& token = parser.peek(-1);

    if (!token.is_keyword()) {
        throw std::runtime_error{"Invalid keyword token"};
    }

    KeywordRule rule{};
    rule.m_TextRegion = token.TextRegion;
    rule.m_Kind = token.Kind;

    return rule;
}

LiteralExprRule LiteralExprRule::create(TextModParser& parser) {

    LiteralExprRule rule{};
    rule.m_TextRegion = parser.peek().TextRegion;

    const bool has_expr_list = parser.has_rule(ParserRuleKind::AssignmentExprListRule);

    if (!has_expr_list) {
        while (!parser.peek(1).is_eolf()) {
            parser.advance();
        }
        rule.m_TextRegion.extend(parser.peek().TextRegion);
    }
    parser.advance();

    return rule;
}

PrimitiveExprRule PrimitiveExprRule::create(TextModParser& par) {
    PrimitiveExprRule rule{};

    auto it = par.create_iterator();

    switch (it->Kind) {
        case Number:
            rule.m_InnerRule = NumberExprRule::create(par);
            break;

        case StringLiteral:
            rule.m_InnerRule = StrExprRule::create(par);
            break;

        default: {
            // Note: Can't match against identifier directly due to overlap in keywords and identifiers
            // i.e., Class is a keyword so Class'Foo' would fail when it shouldn't
            if (it->is_identifier() && it.peek_next() == NameLiteral) {
                rule.m_InnerRule = NameExprRule::create(par);
            }
            // Primarily for: True, False, None; but generically captures any keyword token
            else if (it->is_keyword()) {
                rule.m_InnerRule = KeywordRule::create(par);
            }
            // Unquoted string literals or enumerations
            else {
                rule.m_InnerRule = LiteralExprRule::create(par);
            }
            break;
        }
    }

    return rule;
}

}  // namespace tm_parse::rules