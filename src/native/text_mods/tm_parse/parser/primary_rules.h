//
// Date       : 05/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "parser/parser_rules.h"
#include "parser/primary_expr_rules.h"

namespace tm_parse::rules {

// set Foo'Baz.Bar' Property (1)
class SetCommandRule : public ParserBaseRule {
   private:
    std::variant<ObjectIdentifierRule, NameExprRule> m_Object;
    PropertyAccessRule m_Property;
    ExpressionRule m_Expression;

   public:
    template <class T>
        requires(std::is_same_v<T, ObjectIdentifierRule> || std::is_same_v<T, NameExprRule>)
    bool has_object() const noexcept {
        return std::holds_alternative<T>(m_Object);
    }

    template <class T>
        requires(std::is_same_v<T, ObjectIdentifierRule> || std::is_same_v<T, NameExprRule>)
    const T& object() const noexcept {
        return std::get<T>(m_Object);
    };
    const PropertyAccessRule& property() const noexcept { return m_Property; };
    const ExpressionRule& expr() const noexcept { return m_Expression; };

   public:
    RULE_PUBLIC_API(SetCommandRule);
};

// Begin Object Class=Foo.Baz Name=Foo.Baz:Bar
//   Foo=1
// End Object
//
class ObjectDefinitionRule : public ParserBaseRule {
   private:
    DotIdentifierRule m_Class;
    ObjectIdentifierRule m_Name;
    std::vector<CopyPtr<ObjectDefinitionRule>> m_ChildObjects;
    std::vector<AssignmentExprRule> m_Assignments;

   public: // clang-format off
    const DotIdentifierRule& clazz()                     const noexcept { return m_Class;        }
    const ObjectIdentifierRule& name()                   const noexcept { return m_Name;         }
    const decltype(m_ChildObjects)& child_objects()      const noexcept { return m_ChildObjects; }
    const std::vector<AssignmentExprRule>& assignments() const noexcept { return m_Assignments;  }
    // clang-format on

   public:
    RULE_PUBLIC_API(ObjectDefinitionRule);
};

}  // namespace tm_parse::rules
