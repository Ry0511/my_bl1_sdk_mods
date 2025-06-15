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

        // Skip: (A=,B=)
        if (!parser.peek().is_any<TokenKind::Comma, TokenKind::RightParen>()) {
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

bool AssignmentExprListRule::can_parse(TextModParser& p) {
    using T = TokenKind;
    if (!p.peek().is_identifier()) {
        return false;
    }

    if (p.peek(1) == T::Equal) {
        return true;
    }

    if (p.peek(1).is_any<T::LeftParen, T::LeftBracket>() && p.peek(2) == T::Number
        && p.peek(3).is_any<T::RightParen, T::RightBracket>()) {
        return true;
    }

    return false;
}

ParenExprRule ParenExprRule::create(TextModParser& parser) {
    ParenExprRule rule{};
    parser.require<TokenKind::LeftParen>();
    rule.m_TextRegion = parser.peek(-1).TextRegion;

    //
    // We will need to recursively handle expressions like the following:
    //  > (1)                         -> ParenExpr( NumberExpr )
    //  > ((1))                       -> ParenExpr( ParenExpr( NumberExpr ) )
    //  > (((1)))                     -> ParenExpr( ParenExpr( ParenExpr( NumberExpr ) ) )
    //  > ((A=1,B=(C=2,D=((3))),E=4)) -> ...
    //
    // Downside of this is that it pollutes the result with bloat as the actual inner value is
    // disguised. There really isn't a great to avoid this however we can initially parse as this
    // and then flatten the structure after the fact.
    //

    // A=()
    if (parser.maybe<TokenKind::RightParen>()) {
        rule.m_TextRegion.extend(parser.peek(-1).TextRegion);
    }
    // ( Expression )
    else {
        rule.m_Expr = ExpressionRule::create(parser);
        parser.require<TokenKind::RightParen>();
        rule.m_TextRegion.extend(parser.peek(-1).TextRegion);
    }

    return rule;
}

str_view ExpressionRule::to_string(const TextModParser& parser) const noexcept {
    return text_region().view_from(parser.text());
}

ExpressionRule ExpressionRule::create(TextModParser& parser) {
    using T = TokenKind;
    ExpressionRule rule{};

    const Token& tk = parser.peek();

    if (tk == T::LeftParen) {
        rule.m_InnerType = ParenExprRule::create(parser);
    } else if (AssignmentExprListRule::can_parse(parser)) {
        rule.m_InnerType = AssignmentExprListRule::create(parser);
    } else {
        rule.m_InnerType = PrimitiveExprRule::create(parser);
    }

    return rule;
}

}  // namespace tm_parse::rules