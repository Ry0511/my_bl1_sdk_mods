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
    parser.require<TokenKind::Number>();

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
    const Token& token = parser.peek();
    TXT_MOD_ASSERT((token == TokenKind::StringLiteral), "logic error");

    StrExprRule rule{};
    rule.m_TextRegion = token.TextRegion;
    parser.advance();

    return rule;
}

NameExprRule NameExprRule::create(TextModParser& parser) {

    const Token& token = parser.peek();
    TXT_MOD_ASSERT(token.is_identifier(), "Expecting a Identifier token");

    NameExprRule rule{};
    rule.m_TextRegion = token.TextRegion;

    parser.require_next<TokenKind::NameLiteral>();
    rule.m_TextRegion.extend(parser.peek().TextRegion);
    parser.advance();

    return rule;
}

KeywordRule KeywordRule::create(TextModParser& parser) {

    const Token& token = parser.peek();
    TXT_MOD_ASSERT(token.is_keyword(), "Expecting a keyword token");

    KeywordRule rule{};
    rule.m_TextRegion = token.TextRegion;
    rule.m_Kind = token.Kind;
    parser.advance();

    return rule;
}

LiteralExprRule LiteralExprRule::create(TextModParser& parser) {

    LiteralExprRule rule{};
    rule.m_TextRegion = parser.peek().TextRegion;
    auto secondary = parser.secondary();

    if (secondary == ParserRuleKind::PrimitiveExpr) {
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

    using T = TokenKind;
    const Token& token = par.peek();
    auto secondary = par.secondary();

    switch (token.Kind) {
        case T::Number:
            rule.m_InnerRule = NumberExprRule::create(par);
            break;

        case T::StringLiteral:
            rule.m_InnerRule = StrExprRule::create(par);
            break;

        default: {
            // Note: Can't match against identifier directly due to overlap in keywords and identifiers
            // i.e., Class is a keyword so Class'Foo' would fail when it shouldn't
            if (token.is_identifier() && par.peek(1) == T::NameLiteral) {
                rule.m_InnerRule = NameExprRule::create(par);
            }
            // Primarily for: True, False, None; but generically captures any keyword token
            else if (token.is_keyword()) {
                rule.m_InnerRule = KeywordRule::create(par);
            }

            break;
        }
    }

    // When not parsing as a ParenExpr we can use LiteralExpr we really don't want to use this if we
    // can avoid it but it is the only way to handle sequences like this:
    //
    //   > set Foo Baz My Unquoted String
    //   > Property= My Unquoted String
    //   > (A=My Unquoted String,B=)
    //
    // I have yet to see an unquoted literal inside a ParenExpr and will assume it doesn't exist
    // because thats annoying to handle. Regardless, whatever is returned will be validated by the
    // caller since they can strongly assert what the next token **should** be. We can technically
    // do it here but it complicates things for us and the caller, so dont.
    //
    if (secondary != ParserRuleKind::ParenExpr && rule.is<std::monostate>()) {
        par.set_secondary(ParserRuleKind::PrimitiveExpr);
        rule.m_InnerRule = LiteralExprRule::create(par);
        par.set_secondary(secondary); // Restore
    }

    return rule;
}

}  // namespace tm_parse::rules