//
// Date       : 04/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "primary_expr_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

////////////////////////////////////////////////////////////////////////////////
// | COPYABLE EXPR |
////////////////////////////////////////////////////////////////////////////////

CopyableExpr::CopyableExpr(const ExpressionRule& expr) : m_Expr(std::make_unique<ExpressionRule>(expr)) {}

CopyableExpr::CopyableExpr(const CopyableExpr& other)
    : m_Expr((other == nullptr) ? nullptr : std::make_unique<ExpressionRule>(*other.m_Expr)) {}

CopyableExpr& CopyableExpr::operator=(const CopyableExpr& other) {
    m_Expr = ((other == nullptr) ? nullptr : std::make_unique<ExpressionRule>(*other.m_Expr));
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
// | RULES |
////////////////////////////////////////////////////////////////////////////////

AssignmentExprRule AssignmentExprRule::create(TextModParser& parser) {
    AssignmentExprRule rule{};

    rule.m_Property = PropertyAccessRule::create(parser);
    rule.m_TextRegion = rule.m_Property.text_region();

    parser.require<TokenKind::Equal>();
    rule.m_TextRegion.extend(parser.peek(-1).TextRegion);

    // EOF | BlankLine
    if (parser.peek() != TokenKind::EndOfInput) {
        if (parser.peek() != TokenKind::Comma) {
            rule.m_Expr = ExpressionRule::create(parser);
            rule.m_TextRegion.extend(rule.expr().text_region());
        }
    } else {
        rule.m_Expr = nullptr;
        rule.m_TextRegion.extend(rule.property().text_region());
    }

    return rule;
}

AssignmentExprListRule AssignmentExprListRule::create(TextModParser& parser) {
    AssignmentExprListRule list{};
    list.m_Assignments.reserve(32);  // NOLINT(*-magic-numbers)

    // Initial expression
    list.m_Assignments.push_back(AssignmentExprRule::create(parser));
    list.m_TextRegion = list.m_Assignments.front().text_region();

    // ( Comma AssignmentExprRule )*
    while (parser.maybe<TokenKind::Comma>()) {
        list.m_Assignments.push_back(AssignmentExprRule::create(parser));
    }

    // Extend the text region to encompass the full list of expressions
    if (list.size() > 1) {
        list.m_TextRegion.extend(list[list.size() - 1].text_region());
    }

    list.m_Assignments.shrink_to_fit();
    return list;
}

StructExprRule StructExprRule::create(TextModParser& parser) {
    StructExprRule rule;

    parser.require<TokenKind::LeftParen>();
    rule.m_TextRegion = parser.peek(-1).TextRegion;

    int paren_count = 1;

    while (paren_count > 0 && parser.peek() != TokenKind::EndOfInput) {
        if (parser.peek() == TokenKind::LeftParen) {
            ++paren_count;
        } else if (parser.peek() == TokenKind::RightParen) {
            --paren_count;
        }
        rule.m_TextRegion.extend(parser.peek().TextRegion);
        parser.advance();
    }

    if (paren_count != 0) {
        throw std::runtime_error{std::format(
            "Unbalanced parentheses in expression; Count: {}; Text: '{}'",
            paren_count,
            str{rule.to_string(parser)}
        )};
    }

    rule.m_TextRegion.extend(parser.peek(-1).TextRegion);

    return rule;
}

str_view ExpressionRule::to_string(const TextModParser& parser) const noexcept {
    return text_region().view_from(parser.text());
}

ExpressionRule ExpressionRule::create(TextModParser& parser) {
    using T = TokenKind;
    ExpressionRule rule{};

    if (parser.peek() == T::LeftParen) {
        rule.m_InnerType = StructExprRule::create(parser);
    } else {
        rule.m_InnerType = PrimitiveExprRule::create(parser);
    }

    return rule;
}

}  // namespace tm_parse::rules