//
// Date       : 05/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "parser/parser_rules.h"
#include "parser/primary_expr_rules.h"

namespace tm_parse::rules {

class SetCommandRule : public ParserBaseRule {
   private:
    ObjectIdentifierRule m_Object;
    PropertyAccessRule m_Property;
    ExpressionRule m_Expression;

   public:
    const ObjectIdentifierRule& object() const noexcept { return m_Object; };
    const PropertyAccessRule& property() const noexcept { return m_Property; };
    const ExpressionRule& expr() const noexcept { return m_Expression; };

   public:
    RULE_PUBLIC_API(SetCommandRule);
};

}  // namespace tm_parse::rules
