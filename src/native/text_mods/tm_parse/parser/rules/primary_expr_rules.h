//
// Date       : 04/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "parser/copy_ptr.h"
#include "parser/rules/parser_rules.h"

namespace tm_parse::rules {

class ParenExprRule;
class ExpressionRule;
class PrimitiveExprRule;

// PropertyAccessRule Equal Expression?
class AssignmentExprRule : public ParserBaseRule {
   private:
    PropertyAccessRule m_Property;
    CopyPtr<ExpressionRule> m_Expr;

   public:
    bool has_expr() const noexcept { return m_Expr != nullptr; }

   public:
    const PropertyAccessRule& property() const { return m_Property; };
    const ExpressionRule& expr() const noexcept { return *m_Expr; };

   public:
    RULE_PUBLIC_API(AssignmentExprRule, rules_enum::RuleAssignmentExpr);
};

// AssignmentExpr (Comma AssignmentExpr )*
class AssignmentExprListRule : public ParserBaseRule {
   private:
    std::vector<AssignmentExprRule> m_Assignments;

   public:
    size_t size() const noexcept { return m_Assignments.size(); };
    const AssignmentExprRule& at(size_t index) const { return m_Assignments.at(index); };
    AssignmentExprRule& at(size_t index) { return m_Assignments.at(index); };

   public:
    const AssignmentExprRule& operator[](size_t index) const { return m_Assignments[index]; }
    AssignmentExprRule& operator[](size_t index) { return m_Assignments[index]; }

    // clang-format off
   public:
    decltype(auto) cbegin() const noexcept { return m_Assignments.cbegin(); }
    decltype(auto) cend()   const noexcept { return m_Assignments.cend();   }
    decltype(auto) begin()  noexcept       { return m_Assignments.begin();  }
    decltype(auto) end()    noexcept       { return m_Assignments.end();    }
    // clang-format on

   public:
    RULE_PUBLIC_API(AssignmentExprListRule, rules_enum::RuleAssignmentExprList);
    static bool can_parse(TextModParser& parser);
};

// LeftParen Expression? RightParen
class ParenExprRule : public ParserBaseRule {
   private:
    CopyPtr<ExpressionRule> m_Expr;

   public:
    bool has_expr() const noexcept { return m_Expr != nullptr; }

   public:
    const ExpressionRule& expr() const noexcept { return *m_Expr; }
    ExpressionRule& expr() noexcept { return *m_Expr; }

    const ExpressionRule* inner_most() const noexcept;

   public:
    RULE_PUBLIC_API(ParenExprRule, rules_enum::RuleParenExpr);
};

// Any Expression
class ExpressionRule {
   public:
    using InnerType = std::variant<std::monostate, PrimitiveExprRule, AssignmentExprListRule, ParenExprRule>;

   private:
    InnerType m_InnerType;

   public:
    operator bool() const noexcept { return !std::holds_alternative<std::monostate>(m_InnerType); }

    TokenTextView text_region() const noexcept {
        return std::visit(
            [](auto&& inner) -> TokenTextView {
                using T = std::decay_t<decltype(inner)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    return TokenTextView{};
                } else {
                    return inner.text_region();
                }
            },
            m_InnerType
        );
    }

    str_view to_string(const TextModParser& parser) const noexcept;

    template <class T>
    bool is() const noexcept {
        // clang-format off
        if constexpr (
               std::is_same_v<T, std::monostate>
            || std::is_same_v<T, PrimitiveExprRule>
            || std::is_same_v<T, AssignmentExprListRule>
            || std::is_same_v<T, ParenExprRule>
        ) {
            return std::holds_alternative<T>(m_InnerType);
        } else {
            return std::get<PrimitiveExprRule>(m_InnerType).is<T>();
        }
        // clang-format on
    }

    template <class T>
    const T& get() const noexcept {
        // clang-format off
        if constexpr (
               std::is_same_v<T, std::monostate>
            || std::is_same_v<T, PrimitiveExprRule>
            || std::is_same_v<T, AssignmentExprListRule>
            || std::is_same_v<T, ParenExprRule>
        ) {
            return std::get<T>(m_InnerType);
        } else {
            return std::get<PrimitiveExprRule>(m_InnerType).get<T>();
        }
        // clang-format on
    }

   public:
    RULE_PUBLIC_API(ExpressionRule, rules_enum::RuleExpression);
};

}  // namespace tm_parse::rules
