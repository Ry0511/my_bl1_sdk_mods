//
// Date       : 04/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "primary_expr_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

using namespace tokens_enum;
using namespace rules_enum;

////////////////////////////////////////////////////////////////////////////////
// | RULES |
////////////////////////////////////////////////////////////////////////////////

AssignmentExprRule AssignmentExprRule::create(TextModParser& parser) {
    AssignmentExprRule rule{};

    //
    // Property(0) = Expression
    // ( A=(), B=(1), C=(X=10,Y=20) )
    //    ^^^   ^^^^    ^^^^^^^^^^^

    parser.push_rule(RuleAssignmentExpr);
    rule.m_Property = PropertyAccessRule::create(parser);
    rule.m_TextRegion = rule.m_Property.text_region();

    parser.require<TokenKind::Equal>();
    rule.m_TextRegion.extend(parser.previous().TextRegion);

    auto it = parser.create_iterator();

    // EOF | BlankLine
    if (it.match_seq<EndOfInput>() != 0) {
        // Skip: (A=,B=)
        if (!it->is_any<Comma, RightParen>()) {
            rule.m_Expr = ExpressionRule::create(parser);
            rule.m_TextRegion.extend(rule.expr().text_region());
        }
    } else {
        rule.m_Expr = nullptr;
        rule.m_TextRegion.extend(rule.property().text_region());
    }

    TXT_MOD_ASSERT(parser.peek_rule() == RuleAssignmentExpr);
    parser.pop_rule();

    return rule;
}

AssignmentExprListRule AssignmentExprListRule::create(TextModParser& parser) {
    AssignmentExprListRule list{};
    list.m_Assignments.reserve(32);  // NOLINT(*-magic-numbers)

    parser.push_rule(RuleAssignmentExprList);

    // Initial expression
    list.m_Assignments.push_back(AssignmentExprRule::create(parser));
    list.m_TextRegion = list.m_Assignments.front().text_region();

    // ( Comma AssignmentExprRule )*
    while (parser.maybe<Comma>()) {
        list.m_Assignments.push_back(AssignmentExprRule::create(parser));
    }

    // Extend the text region to encompass the full list of expressions
    if (list.size() > 1) {
        list.m_TextRegion.extend(list[list.size() - 1].text_region());
    }

    list.m_Assignments.shrink_to_fit();
    TXT_MOD_ASSERT(parser.peek_rule() == RuleAssignmentExprList);
    parser.pop_rule();
    return list;
}

bool AssignmentExprListRule::can_parse(TextModParser& p) {
    auto it = p.create_iterator();

    it.set_skip_blank_lines(false);
    it.set_coalesce(true);

    return it.match_seq<Identifier, Equal>() == 0
           || it.match_seq<Identifier, LeftParen, Number, RightParen, Equal>() == 0
           || it.match_seq<Identifier, LeftBracket, Number, RightBracket, Equal>() == 0;
}

const ExpressionRule* ParenExprRule::inner_most() const noexcept {
    if (m_Expr == nullptr) {
        return nullptr;
    }

    // If we hold a ParenExprRule then get what that holds
    if (m_Expr->is<ParenExprRule>()) {
        return m_Expr->get<ParenExprRule>().inner_most();
    }

    // We have an expression
    return m_Expr.get();
}

ParenExprRule ParenExprRule::create(TextModParser& parser) {
    parser.push_rule(RuleParenExpr);
    ParenExprRule rule{};
    parser.require<TokenKind::LeftParen>();

    rule.m_TextRegion = parser.previous().TextRegion;

    //
    // We will need to recursively handle expressions like the following:
    //  > (1)                         -> ParenExpr( NumberExpr )
    //  > ((1))                       -> ParenExpr( ParenExpr( NumberExpr ) )
    //  > (((1)))                     -> ParenExpr( ParenExpr( ParenExpr( NumberExpr ) ) )
    //  > ((A=1,B=(C=2,D=((3))),E=4)) -> ...
    //
    // Downside of this is that it pollutes the result with bloat as the actual inner value is
    // disguised. There really isn't a great to avoid this however we can initially parse as this
    // and then flatten the structure afterwards.
    //

    // A=()
    if (parser.maybe<TokenKind::RightParen>()) {
        rule.m_TextRegion.extend(parser.previous().TextRegion);
    }
    // ( Expression )
    else {
        rule.m_Expr = ExpressionRule::create(parser);
        parser.require<TokenKind::RightParen>();
        rule.m_TextRegion.extend(parser.previous().TextRegion);
    }

    TXT_MOD_ASSERT(parser.peek_rule() == RuleParenExpr);
    parser.pop_rule();
    return rule;
}

str_view ExpressionRule::to_string(const TextModParser& parser) const noexcept {
    return text_region().view_from(parser.text());
}

ExpressionRule ExpressionRule::create(TextModParser& parser) {
    ExpressionRule rule{};

    parser.push_rule(RuleExpression);
    auto it = parser.create_iterator();
    const Token& tk = *it;

    if (it == LeftParen) {
        rule.m_InnerType = ParenExprRule::create(parser);
    } else if (AssignmentExprListRule::can_parse(parser)) {
        rule.m_InnerType = AssignmentExprListRule::create(parser);
    } else {
        rule.m_InnerType = PrimitiveExprRule::create(parser);
    }

    TXT_MOD_ASSERT(parser.peek_rule() == RuleExpression);
    parser.pop_rule();

    return rule;
}

}  // namespace tm_parse::rules