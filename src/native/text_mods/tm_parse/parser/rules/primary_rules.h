//
// Date       : 05/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "parser/rules/parser_rules.h"
#include "parser/rules/primary_expr_rules.h"

namespace tm_parse::rules {

// set Foo'Baz.Bar' Property (1)
class SetCommandRule : public ParserBaseRule {
   private:
    ObjectAccessRule m_Object;
    PropertyAccessRule m_Property;
    ExpressionRule m_Expression;

   public:
    const ObjectAccessRule& object() const noexcept { return m_Object; };
    const PropertyAccessRule& property() const noexcept { return m_Property; };
    const ExpressionRule& expr() const noexcept { return m_Expression; };

   public:
    RULE_PUBLIC_API(SetCommandRule, rules_enum::RuleSetCommand);
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

   public:  // clang-format off
    const DotIdentifierRule& clazz()                     const noexcept { return m_Class;        }
    const ObjectIdentifierRule& name()                   const noexcept { return m_Name;         }
    const decltype(m_ChildObjects)& child_objects()      const noexcept { return m_ChildObjects; }
    const std::vector<AssignmentExprRule>& assignments() const noexcept { return m_Assignments;  }
            // clang-format on

   public:
    RULE_PUBLIC_API(ObjectDefinitionRule, rules_enum::RuleObjectDefinition);
};

// ( SetCommandRule | ObjectDefinitionRule | EOF )*
class ProgramRule {
   public:
    using Inner = std::variant<SetCommandRule, ObjectDefinitionRule>;

   private:
    std::vector<Inner> m_Rules;

   public:
    operator bool() const noexcept { return true; }

   public:
    const std::vector<Inner>& rules() const noexcept { return m_Rules; }

   public:
    template <class T>
    const T& get(size_t index) const {
        if (index >= m_Rules.size()) {
            throw std::out_of_range("index out of range");
        }
        return std::get<T>(m_Rules[index]);
    }

    template <class T>
    T& get(size_t index) {
        if (index >= m_Rules.size()) {
            throw std::out_of_range("index out of range");
        }
        return std::get<T>(m_Rules[index]);
    }

   public:
    template <class T>
    bool has(size_t index) const noexcept {
        return index < m_Rules.size() && std::holds_alternative<T>(m_Rules[index]);
    }

   public:
    RULE_PUBLIC_API(ProgramRule, rules_enum::RuleProgram);
};

}  // namespace tm_parse::rules
