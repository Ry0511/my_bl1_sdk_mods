//
// Date       : 28/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/parser_rules.h"
#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"

namespace tm_parse::rules {

using namespace rules_enum;

str_view ParserBaseRule::to_string(TextModParser& parser) const {
    if (!this->operator bool() || !m_TextRegion.is_valid()) {
        return str_view{};
    }
    return m_TextRegion.view_from(parser.m_Lexer->text());
}

void ParserBaseRule::copy_str_internal(TextModParser& parser) {
    if (!this->operator bool() || !m_TextRegion.is_valid()) {
        throw std::runtime_error{"Cannot copy string internally of invalid rule"};
    }
    m_Text = std::make_shared<str>(m_TextRegion.view_from(parser.text()));
}


////////////////////////////////////////////////////////////////////////////////
// | STATIC FACTORY METHODS |
////////////////////////////////////////////////////////////////////////////////

IdentifierRule IdentifierRule::create(TextModParser& parser) {
    parser.push_rule(RuleIdentifier);

    parser.require<TokenKind::Identifier>();
    IdentifierRule rule{};
    rule.m_TextRegion = parser.peek(-1).TextRegion;

    TXT_MOD_ASSERT(parser.peek_rule() == RuleIdentifier);
    parser.pop_rule();
    return rule;
}

DotIdentifierRule DotIdentifierRule::create(TextModParser& parser) {
    parser.push_rule(RuleDotIdentifier);

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

    TXT_MOD_ASSERT(parser.peek_rule() == RuleDotIdentifier);
    parser.pop_rule();
    return rule;
}

ObjectIdentifierRule ObjectIdentifierRule::create(TextModParser& parser) {
    parser.push_rule(RuleObjectIdentifier);

    ObjectIdentifierRule rule{};
    rule.m_PrimaryIdentifier = DotIdentifierRule::create(parser);
    rule.m_TextRegion = rule.m_PrimaryIdentifier.text_region();

    if (parser.maybe<TokenKind::Colon>()) {
        rule.m_ChildIdentifier = DotIdentifierRule::create(parser);
        rule.m_TextRegion.extend(rule.m_ChildIdentifier.text_region());
    }

    TXT_MOD_ASSERT(parser.peek_rule() == RuleObjectIdentifier);
    parser.pop_rule();
    return rule;
}

ArrayAccessRule ArrayAccessRule::create(TextModParser& parser) {
    parser.push_rule(RuleArrayAccess);
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

    TXT_MOD_ASSERT(parser.peek_rule() == RuleArrayAccess);
    parser.pop_rule();
    return rule;
}

PropertyAccessRule PropertyAccessRule::create(TextModParser& parser) {
    parser.push_rule(RulePropertyAccess);

    PropertyAccessRule rule{};
    rule.m_Identifier = IdentifierRule::create(parser);
    rule.m_TextRegion = rule.m_Identifier.text_region();

    using namespace tokens;

    // Note ambiguity here:
    //   > set Obj Property (1)    | ArrayAccess ?
    //   > set Obj Property(1) (1) | No issues
    //   > set Obj Property ()     | Errors
    //
    // The ambiguity stems from the fact that the (1) part can be either an ArrayAccess or a ParenExpr
    //  we can't know without doing a lookahead to see if there is more tokens to parse or not. This
    //  ambiguity only happens in set commands since other expressions i.e., AssignmentExpr have a
    //  distinct separator token.
    //

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

    const bool maybe_array_access = parser.match_seq<LeftParen, Number, RightParen>() == 0;
    const bool child_of_set_cmd = parser.has_rule(RuleSetCommand);

    // Careful to handle edge cases
    if (maybe_array_access && (!child_of_set_cmd || !parser.peek(3).is_eolf())) {
        rule.m_ArrayAccess = ArrayAccessRule::create(parser);
        rule.m_TextRegion.extend(rule.m_ArrayAccess.text_region());
    }

    TXT_MOD_ASSERT(parser.peek_rule() == RulePropertyAccess);
    parser.pop_rule();
    return rule;
}

}  // namespace tm_parse::rules