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

DotIdentifierRule DotIdentifierRule::create(TextModParser* parser) {
    TXT_MOD_ASSERT(parser->peek() == TokenKind::Identifier, "parser error");
    DotIdentifierRule rule{};

    rule.StartIndex = parser->top();

    // While we have a dot we must have an identifier
    while (parser->maybe_next(TokenKind::Dot)) {
        parser->expect(TokenKind::Identifier);
    }

    rule.EndIndex = parser->top();

    return rule;
}

ObjectIdentifierRule ObjectIdentifierRule::create(TextModParser* parser) {
    ObjectIdentifierRule rule{};

    rule.PrimaryIdentifier = DotIdentifierRule::create(parser);

    // Contains a child identifier
    if (parser->maybe_next(TokenKind::Colon)) {
        parser->expect(TokenKind::Identifier);  // Required after a colon
        rule.SecondaryIdentifier = DotIdentifierRule::create(parser);
    }

    return rule;
}

str SetCommandRule::as_str(TextModParser* parser) const noexcept {
    str_view full_text = parser->m_Lexer->text();

    size_t start = parser->m_Regions[SetCommandIndex].Start;
    auto end = parser->m_Regions[PropertyIndex];

    str_view cmd_text = full_text.substr(start, (end.Start + end.Length) - start);
    return str{cmd_text};
}

SetCommandRule SetCommandRule::create(TextModParser* parser) {
    SetCommandRule rule{};

    rule.SetCommandIndex = parser->top();
    parser->expect(TokenKind::Identifier);
    rule.ObjectIdentifier = ObjectIdentifierRule::create(parser);
    parser->expect(TokenKind::Identifier);
    rule.PropertyIndex = parser->top();
    rule.CompositeExpr = CompositeExprRule{};  // Default

    return rule;
}

}  // namespace tm_parse::rules