//
// Date       : 02/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/rules/primitive_expr_rules.h"
#include "parser/rules/parser_rules.h"
#include "parser/text_mod_parser.h"

namespace tm_parse::rules {

using namespace txt;
using namespace tokens_enum;
using namespace rules_enum;

NumberExprRule NumberExprRule::create(TextModParser& parser) {
    parser.push_rule(RuleNumberExpr);
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

    TXT_MOD_ASSERT(parser.peek_rule() == RuleNumberExpr);
    parser.pop_rule();

    return rule;
}

StrExprRule StrExprRule::create(TextModParser& parser) {
    parser.push_rule(RuleStrExpr);

    parser.require<StringLiteral>();
    StrExprRule rule{};
    const Token& token = parser.peek(-1);
    rule.m_TextRegion = token.TextRegion;

    TXT_MOD_ASSERT(parser.peek_rule() == RuleStrExpr);
    parser.pop_rule();

    return rule;
}

NameExprRule NameExprRule::create(TextModParser& parser) {
    parser.push_rule(RuleNameExpr);

    NameExprRule rule{};

    rule.m_Class = DotIdentifierRule::create(parser);
    rule.m_TextRegion = rule.m_Class->text_region();

    parser.require<TokenKind::NameLiteral>();
    auto literal_region = parser.peek(-1).TextRegion;
    rule.m_TextRegion.extend(literal_region);

    // Hack because I am lazy and don't want to mess with the lexer; -2 for both single quotes
    // TODO: Fixme this needs to offset Start
    auto text = parser.text().substr(literal_region.Start + 1, literal_region.Length - 2);
    TextModLexer temp_lexer{text};
    TextModParser temp_parser{&temp_lexer};
    rule.m_Identifier = ObjectIdentifierRule::create(temp_parser);

    TXT_MOD_ASSERT(parser.peek_rule() == RuleNameExpr);
    parser.pop_rule();

    return rule;
}

KeywordRule KeywordRule::create(TextModParser& parser) {
    parser.push_rule(RuleKeyword);

    parser.require<Identifier>(0, {.Coalesce = true, .SkipOnBlankLine = false});
    const Token& token = parser.peek(-1);

    if (!token.is_keyword()) {
        throw std::runtime_error{"Invalid keyword token"};
    }

    KeywordRule rule{};
    rule.m_TextRegion = token.TextRegion;
    rule.m_Kind = token.Kind;

    TXT_MOD_ASSERT(parser.peek_rule() == RuleKeyword);
    parser.pop_rule();

    return rule;
}

LiteralExprRule LiteralExprRule::create(TextModParser& parser) {
    parser.push_rule(RuleLiteralExpr);

    LiteralExprRule rule{};

    if (parser.peek() == EndOfInput) {
        throw std::runtime_error{"Invalid literal expression"};
    }

    while (parser.peek() == BlankLine) {
        parser.advance();
    }

    rule.m_TextRegion = parser.peek().TextRegion;

    // Only want to consume the entire line if the current tree does not include a ParenExpr
    const bool consume_to_end = !parser.has_rule<RuleObjectDefinition, RuleSetCommand>(RuleParenExpr);

    if (consume_to_end) {
        while (!parser.peek(1).is_eolf()) {
            parser.advance();
        }
        rule.m_TextRegion.extend(parser.peek().TextRegion);
    }
    parser.advance();

    TXT_MOD_ASSERT(parser.peek_rule() == RuleLiteralExpr);
    parser.pop_rule();

    return rule;
}

PrimitiveExprRule PrimitiveExprRule::create(TextModParser& par) {
    PrimitiveExprRule rule{};

    par.push_rule(RulePrimitiveExpr);
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

    TXT_MOD_ASSERT(par.peek_rule() == RulePrimitiveExpr);
    par.pop_rule();
    return rule;
}

}  // namespace tm_parse::rules