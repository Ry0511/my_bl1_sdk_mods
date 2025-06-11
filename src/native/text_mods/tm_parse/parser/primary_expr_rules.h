//
// Date       : 04/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "parser/parser_rules.h"

namespace tm_parse::rules {

class StructExprRule;
class ExpressionRule;
class PrimitiveExprRule;

class AssignmentExprRule : public ParserBaseRule {
   private:
    PropertyAccessRule m_Property;
    std::unique_ptr<ExpressionRule> m_Expression;

   public:
    AssignmentExprRule(const AssignmentExprRule& rule);
    AssignmentExprRule& operator=(const AssignmentExprRule& rule);
    AssignmentExprRule(AssignmentExprRule&& rule) = default;
    AssignmentExprRule& operator=(AssignmentExprRule&& rule) = default;

   public:
    const PropertyAccessRule& property() const { return m_Property; };
    const ExpressionRule& expr() const noexcept { return *m_Expression; };

   public:
    RULE_PUBLIC_API(AssignmentExprRule);
};

// ( PropertyAssign ( Comma PropertyAssign )* )
class StructExprRule : public ParserBaseRule {
   private:
    std::vector<AssignmentExprRule> m_Assignments;

   public:
    RULE_PUBLIC_API(StructExprRule);
};

// Any Expression
class ExpressionRule : public ParserBaseRule {
   public:
    using InnerType = std::variant<std::monostate, PrimitiveExprRule, StructExprRule>;

   private:
    InnerType m_InnerType;

   public:
    operator bool() const noexcept { return !std::holds_alternative<std::monostate>(m_InnerType); }

    template <class T>
    bool is() const noexcept {
        return std::holds_alternative<T>(m_InnerType);
    }

    template <class T>
    const T& get() const noexcept {
        return std::get<T>(m_InnerType);
    }

   public:
    RULE_PUBLIC_API(ExpressionRule);
};

}  // namespace tm_parse::rules
