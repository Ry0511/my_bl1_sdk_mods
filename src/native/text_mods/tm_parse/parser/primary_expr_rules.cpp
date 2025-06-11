//
// Date       : 04/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "primary_expr_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

AssignmentExprRule::AssignmentExprRule(const AssignmentExprRule& rule)
    : m_Property(rule.m_Property),
      m_Expression(std::make_unique<ExpressionRule>(*rule.m_Expression)) {}

AssignmentExprRule& AssignmentExprRule::operator=(const AssignmentExprRule& rule) {
    m_TextRegion = rule.m_TextRegion;
    m_Property = rule.m_Property;
    m_Expression = std::make_unique<ExpressionRule>(*rule.m_Expression);
    return *this;
}

AssignmentExprRule AssignmentExprRule::create(TextModParser& parser) {
    AssignmentExprRule rule;
    rule.m_TextRegion = parser.peek().TextRegion;
    rule.m_Property = PropertyAccessRule::create(parser);
    rule.m_Expression = std::make_unique<ExpressionRule>(ExpressionRule::create(parser));
    rule.m_TextRegion.extend(rule.m_Expression->text_region());
    return rule;
}

StructExprRule StructExprRule::create(TextModParser& parser) {
    TXT_MOD_ASSERT(parser.peek() == TokenKind::LeftParen, "logic error");

    StructExprRule rule;
    rule.m_TextRegion = parser.peek().TextRegion;
    parser.advance();

    bool has_next = false;
    do {
        AssignmentExprRule expr{};
        rule.m_Assignments.push_back(expr);
        parser.require_next<TokenKind::RightParen, TokenKind::Comma>();
        has_next = (parser.peek() == TokenKind::Comma);
    } while (has_next);

    rule.m_TextRegion.extend(parser.peek().TextRegion);

    return rule;
}

ExpressionRule ExpressionRule::create(TextModParser& parser) {
    using T = TokenKind;
    ExpressionRule rule{};

    // TODO: This is wrong use current token
    if (parser.maybe_next<T::LeftParen>()) {
        rule.m_TextRegion = parser.peek().TextRegion;

        // A=()
        if (parser.maybe_next<T::RightParen>()) {
            rule.m_InnerType = std::monostate{};
            parser.advance();
        }
        // Note: This isn't an actual value people will use. It is just the result of dumping an
        //       invalid enum value from an object.
        // A=(INVALID)
        else if (parser.maybe_next<T::Kw_Invalid>()) {
            rule.m_InnerType = std::monostate{};
            parser.require_next<T::RightParen>();
            parser.advance();
        }
        // A=(X=10, Y=20, Z=30, W=40)
        else {
            rule.m_InnerType = StructExprRule::create(parser);
        }

        rule.m_TextRegion.extend(parser.peek().TextRegion);

    }
    // We can only assume its a primitive value
    else {
        rule.m_TextRegion = parser.peek().TextRegion;
        rule.m_InnerType = PrimitiveExprRule::create(parser);
        rule.m_TextRegion.extend(parser.peek(-1).TextRegion);
    }

    return rule;
}

}  // namespace tm_parse::rules