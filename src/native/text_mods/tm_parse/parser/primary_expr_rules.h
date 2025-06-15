//
// Date       : 04/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "parser/parser_rules.h"

namespace tm_parse::rules {

class ParenExprRule;
class ExpressionRule;
class PrimitiveExprRule;

class CopyableExpr {
   private:
    std::unique_ptr<ExpressionRule> m_Expr;

   public:
    constexpr CopyableExpr(nullptr_t) noexcept : m_Expr(nullptr) {}
    constexpr CopyableExpr() = default;
    ~CopyableExpr() = default;

   public:  // clang-format off
    CopyableExpr(const ExpressionRule& expr);

    CopyableExpr(const CopyableExpr&);
    CopyableExpr& operator=(const CopyableExpr&);

    CopyableExpr(CopyableExpr&&) = default;
    CopyableExpr& operator=(CopyableExpr&&) = default;

   public:
    bool operator ==(const CopyableExpr&) const = default;
    bool operator !=(const CopyableExpr&) const = default;

   public:
    const ExpressionRule* get() const noexcept { return m_Expr.get(); }
    ExpressionRule*       get()       noexcept { return m_Expr.get(); }

   public:
    const ExpressionRule* operator->() const { return m_Expr.get(); }
    ExpressionRule*       operator->()       { return m_Expr.get(); }
    const ExpressionRule& operator*() const  { return *m_Expr;      }
    ExpressionRule&       operator*()        { return *m_Expr;      }
    // clang-format on
};

// PropertyAccessRule Equal Expression?
class AssignmentExprRule : public ParserBaseRule {
   private:
    PropertyAccessRule m_Property;
    CopyableExpr m_Expr;

   public:
    bool has_expr() const noexcept { return m_Expr != nullptr; }

   public:
    const PropertyAccessRule& property() const { return m_Property; };
    const ExpressionRule& expr() const noexcept { return *m_Expr; };

   public:
    RULE_PUBLIC_API(AssignmentExprRule);
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
    RULE_PUBLIC_API(AssignmentExprListRule);
    static bool can_parse(TextModParser& parser);
};

// LeftParen Expression? RightParen
class ParenExprRule : public ParserBaseRule {
   private:
    CopyableExpr m_Expr;

   public:
    RULE_PUBLIC_API(ParenExprRule);
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
