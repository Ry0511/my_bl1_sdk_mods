//
// Date       : 02/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"
#include "parser/parser_rule_enum.h"

namespace tm_parse::rules {

// - NOTE -
// Primitive rules encapsulate the single unit values which can be placed on the right hand side of
// an assignment or as the final expression in a set command. The notion of primitive here is that
// they do not require recursive parsing and are generally single unit tokens. Such as Numbers,
// String Literals, Name Literals, etc. With the exception being Literals which is a little more
// complicated.

class NumberExprRule;   // Number
class StrExprRule;      // \".*?\"
class NameExprRule;     // Identifier NameLiteral
class KeywordRule;      // True | False | None
class LiteralExprRule;  // * -> Terminator

using PrimitiveRuleVariant =
    std::variant<std::monostate, NumberExprRule, StrExprRule, NameExprRule, KeywordRule, LiteralExprRule>;

////////////////////////////////////////////////////////////////////////////////
// | PRIMITIVE RULES |
////////////////////////////////////////////////////////////////////////////////

class NumberExprRule : public ParserBaseRule {
   private:  // Probably enough bits
    std::variant<std::monostate, int32_t, float> m_Value;

   public:
    template <class T>
        requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
    T get() const noexcept(true) {
        return std::visit(
            [](auto&& value) -> T {
                using HeldType = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<HeldType, std::monostate>) {
                    return T{};  // Default initialise
                } else {
                    return static_cast<T>(value);
                }
            },
            m_Value
        );
    }

    operator bool() const noexcept(true) { return !std::holds_alternative<std::monostate>(m_Value); }

   public:
    RULE_PUBLIC_API(NumberExprRule);
};

class StrExprRule : public ParserBaseRule {
   public:
    RULE_PUBLIC_API(StrExprRule);
};

class NameExprRule : public ParserBaseRule {
   public:
    RULE_PUBLIC_API(NameExprRule);
};

class KeywordRule : public ParserBaseRule {
   private:
    TokenKind m_Kind{TokenKind::TokenKind_Count};

   public:
    template <TokenKind... Kinds>
    bool is() const noexcept {
        return (... || (m_Kind == Kinds));
    }

   public:
    RULE_PUBLIC_API(KeywordRule);
};

/**
 * This is a relatively annoying rule but it either consumes exactly one token or as many tokens as
 * it can until it reaches a terminating token. This rule exists primarily to handle unquoted string
 * literals such as:
 *
 *   > set Foo Baz My Unquoted String
 *   > Property=My Unquoted String
 *
 * However, things can get a little more complicated depending on the context such as when parsing
 * from a ParenExpr in which case we can't rely on the (\n | EOF) sequence as this wouldn't work.
 *
 *   > set Foo Baz (A=My Unquoted String, B=My Unquoted String)
 *   > Property=(A=My Unquoted String, B=My Unquoted String)
 *
 * Now, I don't actually believe that unquoted literals are allowed in ParenExpr to begin with and
 * I am hoping and relying on that being the case. Since dealing with it is a PITA for example how
 * do you parse:
 *
 * > Property=(A=My, Unquoted, String,, A=10.0)
 *
 * For this reason alone I am strongly assuming that it is just not allowed. Anyway, all that to say
 * that internal parsing of this rule relies on parental context which is provided to the parser.
 *
 */
class LiteralExprRule : public ParserBaseRule {
   public:
    RULE_PUBLIC_API(LiteralExprRule);
};

// [[ParserDoc_PrimitiveExpr]]
class PrimitiveExprRule {
   private:
    PrimitiveRuleVariant m_InnerRule;

   public:
    operator bool() const noexcept(true) { return !std::holds_alternative<std::monostate>(m_InnerRule); }

   public:
    TokenTextView text_region() const noexcept {
        return std::visit(
            [](auto&& val) -> const TokenTextView& {
                using U = std::decay_t<decltype(val)>;
                if constexpr (std::is_base_of_v<ParserBaseRule, U>) {
                    return val.text_region();
                } else {
                    return TokenTextView{};  // Invalid range
                }
            },
            m_InnerRule
        );
    }

   public:
    template <class T>
    bool is() const noexcept {
        return std::holds_alternative<T>(m_InnerRule);
    }

    template <class T>
    const T& get() const noexcept {
        TXT_MOD_ASSERT(std::holds_alternative<T>(m_InnerRule), "logic error");
        return std::get<T>(m_InnerRule);
    }

   public:
    RULE_PUBLIC_API(PrimitiveExprRule);
};

}  // namespace tm_parse::rules