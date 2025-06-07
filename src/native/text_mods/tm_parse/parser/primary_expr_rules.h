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
class AssignmentExprRule;

// ( PropertyAssign ( Comma PropertyAssign )* )
class StructExprRule : public ParserBaseRule {};

// Any Expression
class ExpressionRule : public ParserBaseRule {
   public:
    // std::variant<std::monostate, PrimitiveExprRule, StructExprRule>;

   private:
   public:
    RULE_PUBLIC_API(ExpressionRule);
};

// Property(0) = My Value Expression
class AssignmentExprRule : public ParserBaseRule {
   private:
    PropertyAccessRule m_Property;
    ExpressionRule m_Expr;

   public:
    RULE_PUBLIC_API(AssignmentExprRule);
};

}  // namespace tm_parse::rules
