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

IdentifierRule IdentifierRule::create(TextModParser& parser) {
    parser.require<TokenKind::Identifier>();
    IdentifierRule rule{};
    rule.m_TextRegion = parser.peek(-1).TextRegion;
    return rule;
}

DotIdentifierRule DotIdentifierRule::create(TextModParser& parser) {
    parser.require<TokenKind::Identifier>();
    DotIdentifierRule rule{};
    rule.m_TextRegion = parser.peek(-1).TextRegion;

    constexpr TextModParser::PeekOptions opt{.SkipOnBlankLine = false};

    bool extend_view = false;
    while (parser.maybe<TokenKind::Dot>(0, opt)) {
        parser.require<TokenKind::Identifier>(0, opt);
        extend_view = true;
    }

    // We read more than a single identifier
    if (extend_view) {
        rule.m_TextRegion.extend(parser.peek(-1).TextRegion);
    }

    return rule;
}

ObjectIdentifierRule ObjectIdentifierRule::create(TextModParser& parser) {
    ObjectIdentifierRule rule{};
    rule.m_PrimaryIdentifier = DotIdentifierRule::create(parser);
    rule.m_TextRegion = rule.m_PrimaryIdentifier.text_region();

    if (parser.maybe<TokenKind::Colon>()) {
        rule.m_ChildIdentifier = DotIdentifierRule::create(parser);
        rule.m_TextRegion.extend(rule.m_ChildIdentifier.text_region());
    } else {
        parser.advance();
    }

    return rule;
}

ArrayAccessRule ArrayAccessRule::create(TextModParser& parser) {
    const Token& token = parser.peek();

    TXT_MOD_ASSERT((token.is_any<TokenKind::LeftBracket, TokenKind::LeftParen>()), "logic error");

    ArrayAccessRule rule{};

    rule.m_TextRegion = parser.peek().TextRegion;

    // ( Number )
    if (parser.maybe<TokenKind::LeftParen>()) {
        rule.m_Index = NumberExprRule::create(parser);
        parser.require<TokenKind::RightParen>();
        rule.m_TextRegion.extend(parser.peek(-1).TextRegion);
    }
    // [ Number ]
    else if (parser.maybe<TokenKind::LeftBracket>()) {
        rule.m_Index = NumberExprRule::create(parser);
        parser.require<TokenKind::RightBracket>();
        rule.m_TextRegion.extend(parser.peek(-1).TextRegion);
    } else {
        throw std::runtime_error(std::format("Expecting ( or [ but got {}", token.as_str()));
    }

    return rule;
}

PropertyAccessRule PropertyAccessRule::create(TextModParser& parser) {
    PropertyAccessRule rule{};
    rule.m_Identifier = IdentifierRule::create(parser);
    rule.m_TextRegion = rule.m_Identifier.text_region();

    using namespace tokens;

    // Note ambiguity here:
    //   > set Obj Property (1)    | ArrayAccess?
    //   > set Obj Property(1) (1) | No issues
    //   > set Obj Property ()     | Errors
    //
    // Should only apply to set commands

    //           set Obj Property(1) (1)
    // peek(0) ->                ^   ^
    // peek(1) ->                 ^  ^
    // peek(2) ->                  ^ ^
    // peek(3) ->                    ^

    // Shouldn't be any edge cases here
    if (parser.match_seq<LeftBracket, Number, RightBracket>() == 0) {
        rule.m_ArrayAccess = ArrayAccessRule::create(parser);
        rule.m_TextRegion.extend(rule.m_ArrayAccess.text_region());
    }

    // No array expression
    if (parser.match_seq<LeftParen, Number, RightParen>() != 0) {
        return rule;
    }

    // If something other than
    if (parser.primary() != ParserRuleKind::SetCommand || !parser.peek(3).is_eolf()) {
        rule.m_ArrayAccess = ArrayAccessRule::create(parser);
        rule.m_TextRegion.extend(rule.m_ArrayAccess.text_region());
    }

    return rule;
}

}  // namespace tm_parse::rules