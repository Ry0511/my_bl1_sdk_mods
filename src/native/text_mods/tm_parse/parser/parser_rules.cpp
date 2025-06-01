//
// Date       : 28/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/parser_rules.h"
#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"

namespace tm_parse::rules {

str_view ParserBaseRule::to_string(TextModParser& parser) const {
    if (!this->operator bool()) {
        return str_view{};
    }
    return m_TextRegion.view_from(parser.m_Lexer->text());
}

////////////////////////////////////////////////////////////////////////////////
// | STATIC FACTORY METHODS |
////////////////////////////////////////////////////////////////////////////////

// - NOTE -
// All parser factory methods must assert the following
//  1. The current token is the entry token for its rule, that is, peek() == EntryToken
//  2. The current token after create should be the last token of the current rule
//  3. The parser position on error is the same as when entering, that is, peek() == EntryToken
//

DotIdentifierRule DotIdentifierRule::create(TextModParser& parser) {
    TXT_MOD_ASSERT(parser.peek() == TokenKind::Identifier, "logic error");

    DotIdentifierRule rule{};
    rule.m_StartTokenIndex = parser.index();
    rule.m_TextRegion = parser.peek().TextRegion;

    while (parser.maybe_next<TokenKind::Dot>()) {      // Advances to Dot
        parser.require_next<TokenKind::Identifier>();  // Advances to Identifier
        rule.m_EndTokenIndex = parser.index();
    }

    // We read more than a single identifier
    if (rule.m_EndTokenIndex != invalid_index_v) {
        const Token& tok = parser.peek();
        rule.m_TextRegion.extend(tok.TextRegion);
    }

    return rule;
}

ObjectIdentifierRule ObjectIdentifierRule::create(TextModParser& parser) {
    TXT_MOD_ASSERT(parser.peek() == TokenKind::Identifier, "logic error");

    ObjectIdentifierRule rule{};
    rule.m_PrimaryIdentifier = DotIdentifierRule::create(parser);
    rule.m_TextRegion = rule.m_PrimaryIdentifier.text_region();

    if (parser.maybe_next<TokenKind::Colon>()) {
        parser.require_next<TokenKind::Identifier>();
        rule.m_ChildIdentifier = DotIdentifierRule::create(parser);
        rule.m_TextRegion.extend(rule.m_ChildIdentifier.text_region());
    }

    return rule;
}

ArrayAccessRule ArrayAccessRule::create(TextModParser& parser) {
    const Token& token = parser.peek();

    TXT_MOD_ASSERT((token.is_any<TokenKind::LeftBracket, TokenKind::LeftParen>()), "logic error");

    ArrayAccessRule rule{};
    rule.m_StartTokenIndex = parser.index();
    rule.m_TextRegion = parser.peek().TextRegion;

    parser.require_next<TokenKind::Number>();

    // ( Number )
    if (token == TokenKind::LeftParen) {
        parser.require_next<TokenKind::RightParen>();

        // [ Number ]
    } else if (token == TokenKind::LeftBracket) {
        parser.require_next<TokenKind::RightBracket>();
    }

    return rule;
}

PropertyAccessRule PropertyAccessRule::create(TextModParser& parser) {
    TXT_MOD_ASSERT(parser.peek() == TokenKind::Identifier, "logic error");

    PropertyAccessRule rule{};
    rule.m_StartTokenIndex = parser.index();
    rule.m_TextRegion = parser.peek().TextRegion;

    // Identifier ArrayAccess?
    if (parser.maybe_next<TokenKind::LeftBracket, TokenKind::LeftParen>()) {
        rule.m_ArrayAccess = ArrayAccessRule::create(parser);
        rule.m_TextRegion.extend(rule.m_ArrayAccess.text_region());
    }

    return rule;
}

PrimitiveExprRule PrimitiveExprRule::create(TextModParser& parser) {
    PrimitiveExprRule rule{};
    return rule;
}

ParenExprRule ParenExprRule::create(TextModParser& parser) {
    ParenExprRule rule{};
    return rule;
}

CompositeExprRule CompositeExprRule::create(TextModParser& parser) {
    CompositeExprRule rule{};
    return rule;
}

SetCommandRule SetCommandRule::create(TextModParser& parser) {
    TXT_MOD_ASSERT(parser.peek() == TokenKind::Kw_Set, "logic error");

    // Kw_Set ObjectIdentifier PropertyAccess ValueExpr
    SetCommandRule rule{};
    rule.m_Parser = &parser;
    rule.m_StartTokenIndex = parser.index();
    rule.m_TextRegion = parser.peek().TextRegion;

    parser.require_next<TokenKind::Identifier>();
    rule.m_ObjectIdentifier = ObjectIdentifierRule::create(parser);

    parser.require_next<TokenKind::Identifier>();
    rule.m_PropertyAccess = PropertyAccessRule::create(parser);

    // TODO: ValueExpr doesn't exist yet so replace this at some point
    rule.m_TextRegion.extend(rule.m_PropertyAccess.text_region());

    return rule;
}

}  // namespace tm_parse::rules