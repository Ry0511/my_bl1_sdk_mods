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
      m_Expression(
          rule.m_Expression != nullptr ? std::make_unique<ExpressionRule>(*rule.m_Expression)
                                       : std::unique_ptr<ExpressionRule>{}
      ) {}

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
    StructExprRule rule;
    rule.m_TextRegion = parser.peek().TextRegion;
    parser.require<TokenKind::LeftParen>();

    int open_count = 1;

    while (open_count > 0 && parser.peek() != TokenKind::EndOfInput) {

        if (parser.peek() == TokenKind::LeftParen) {
            ++open_count;
        } else if (parser.peek() == TokenKind::RightParen) {
            --open_count;
        }

        parser.advance();
    }

    if (open_count < 0 || parser.peek(-1) != TokenKind::RightParen) {
        throw std::runtime_error{std::format("Unbalanced parentheses in expression {}", open_count)};
    }

    rule.m_TextRegion.extend(parser.peek(-1).TextRegion);

    return rule;
}

ExpressionRule ExpressionRule::create(TextModParser& parser) {
    using T = TokenKind;
    ExpressionRule rule{};

    if (parser.peek() == T::LeftParen) {
        rule.m_TextRegion = parser.peek().TextRegion;

        // A=()
        if (parser.maybe_next<T::RightParen>()) {
            parser.advance();
            rule.m_InnerType = std::monostate{};
        }
        // Note: This isn't an actual value people will use. It is just the result of dumping an
        //       invalid enum value from an object. Really shouldn't allow it...
        // A=(INVALID)
        else if (parser.maybe_next<T::Kw_Invalid>()) {
            parser.advance();
            parser.require<T::RightParen>();
            rule.m_InnerType = std::monostate{};
        }
        // A=(X=10, Y=20, Z=30, W=40)
        else {
            rule.m_InnerType = StructExprRule::create(parser);
            parser.advance();
        }

        rule.m_TextRegion.extend(parser.peek(-1).TextRegion);
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