//
// Date       : 28/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/parser_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

// - NOTE -
// All parser rules assume the initial token is at the top of the stack this is asserted. So if we
// are parsing an DotIdentifierRule then (parser->peek() == TokenKind::Identifier) this applies to
// all rules.
//
// Additionally, note that this is a manual recursive descent parser.
//

////////////////////////////////////////////////////////////////////////////////
// | IDENTIFIERS |
////////////////////////////////////////////////////////////////////////////////

DotIdentifierRule DotIdentifierRule::create(TextModParser* parser) {
    TXT_MOD_ASSERT(parser->peek() == TokenKind::Identifier, "parser error");
    DotIdentifierRule rule{};

    rule.m_StartIndex = parser->top();

    // While we have a dot we must have an identifier
    while (parser->maybe_next(TokenKind::Dot)) {
        parser->expect(TokenKind::Identifier);
    }

    rule.m_EndIndex = parser->top();

    return rule;
}

ObjectIdentifierRule ObjectIdentifierRule::create(TextModParser* parser) {
    ObjectIdentifierRule rule{};

    rule.m_PrimaryIdentifier = DotIdentifierRule::create(parser);

    // Contains a child identifier
    if (parser->maybe_next(TokenKind::Colon)) {
        parser->expect(TokenKind::Identifier);  // Required after a colon
        rule.m_SecondaryIdentifier = DotIdentifierRule::create(parser);
    }

    return rule;
}

DotIdentifierRule::operator bool() const noexcept(true) {
    return m_StartIndex != invalid_index_v && m_EndIndex != invalid_index_v;
}

ArrayAccessRule ArrayAccessRule::create(TextModParser* parser) {
    ArrayAccessRule rule{};

    using TokenKind::LeftBracket;
    using TokenKind::LeftParen;
    using TokenKind::RightBracket;
    using TokenKind::RightParen;

    rule.m_OpenTokenIndex = parser->top();
    TokenKind opening_token = parser->m_Tokens[rule.m_OpenTokenIndex];
    TXT_MOD_ASSERT(opening_token == LeftParen || opening_token == LeftBracket, "parser error");

    parser->expect(TokenKind::Number);

    if (opening_token == LeftParen) {
        parser->expect(RightParen);
    } else if (opening_token == LeftBracket) {
        parser->expect(RightBracket);
    }

    TXT_MOD_ASSERT(
        parser->top() == rule.close_token_index(),
        "Expecting {} == {}",
        parser->top(),
        rule.close_token_index()
    );

    return rule;
}

PropertyAccessRule PropertyAccessRule::create(TextModParser* parser) {
    PropertyAccessRule rule{};

    TXT_MOD_ASSERT(parser->peek() == TokenKind::Identifier, "parser error");
    rule.IdentifierTokenIndex = parser->top();

    parser->m_Lexer->save();
    Token token{};
    if (parser->m_Lexer->read_token(&token)) {
        if (token == TokenKind::LeftBracket || token == TokenKind::LeftParen) {
            parser->push_token(token);
            rule.ArrayAccess = ArrayAccessRule::create(parser);
        } else {
            parser->m_Lexer->restore();
        }
    }

    return rule;
}

////////////////////////////////////////////////////////////////////////////////
// | SET COMMAND |
////////////////////////////////////////////////////////////////////////////////

SetCommandRule SetCommandRule::create(TextModParser* parser) {
    TXT_MOD_ASSERT(parser->peek() == TokenKind::Kw_Set, "parser error");
    SetCommandRule rule{};
    rule.m_SetCommandIndex = parser->top();

    parser->expect(TokenKind::Identifier);
    rule.m_ObjectIdentifier = ObjectIdentifierRule::create(parser);

    parser->expect(TokenKind::Identifier);
    rule.m_PropertyAccess = PropertyAccessRule::create(parser);

    // Value Expression
    rule.m_CompositeExpr = CompositeExprRule{};

    return rule;
}

}  // namespace tm_parse::rules