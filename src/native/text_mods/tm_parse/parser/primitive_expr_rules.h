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
// an assignment or as the final expression in a set command.
//
// Now with that said there are a few issues with trying to parse an expression mainly ambiguity,
// that is, some expressions are impossible to deduce without knowing the underlying property type
// being assigned.
//
// i.e.,
//
//  > LastLocationStatusUpdateString=
//  > WeaponHandPreferWeaponHand    = HAND_Rightence=HAND_Right
//  > bResultsScreenOpen            = False
//  > QuickSaveFileName             = QuickSave.sav
//  > NoPauseMessage                = Game is not pauseable
//  > ForceFeedbackManagerClassName = WinDrv.XnaForceFeedbackManager
//  > ForceFeedbackManager          = XnaForceFeedbackManager'menumap.TheWorld:PersistentLevel.'
//
// A few things to note here firstly many different 'rules' start with an identifier which may
// require a lookahead
//
// There is an ambiguity where `QuickSave.sav` and `WinDrv.XnaForceFeedbackManager` could both be
// deduced as a DotIdentifierRule. However, we know without a doubt that `QuickSave.sav` is a string
// but knowing that at the parser level is impossible without knowing the type its being assigned to.
//
// So the solution is simple... Don't allow it. If we can't match the rule to any known primitive
// expression we will raise an exception. This isn't the ideal solution and infact it goes against
// what I want for this project.
//

// Note: EmptyExprRule does not exist since that requires primary knowledge and should be handled
//        there, that is, =\n or =, what token should be deemed as the 'End' token? (The invoker will
//        know this and can trivially check for it)

class NumberExprRule;      // = Number
class BoolExprRule;        // = True | False
class NoneExprRule;        // = None
class StrExprRule;         // = \".*?\"
class NameExprRule;        // = Identifier NameLiteral
class IdentifierExprRule;  // = Identifier

// clang-format off (no way of making this look good)
using PrimitiveRuleVariant =
    std::variant<std::monostate, NumberExprRule, BoolExprRule, NoneExprRule, StrExprRule, NameExprRule>;
// clang-format on

////////////////////////////////////////////////////////////////////////////////
// | PRIMITIVE RULES |
////////////////////////////////////////////////////////////////////////////////

class NumberExprRule : public ParserBaseRule {
   private:
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

class BoolExprRule : public ParserBaseRule {
   private:
    bool m_Value{false};

   public:
    bool get() const noexcept(true) { return m_Value; }

   public:
    RULE_PUBLIC_API(BoolExprRule);
};

class NoneExprRule : public ParserBaseRule {
   public:
    RULE_PUBLIC_API(NoneExprRule);
};

class StrExprRule : public ParserBaseRule {
   public:
    RULE_PUBLIC_API(StrExprRule);
};

class NameExprRule : public ParserBaseRule {
   public:
    RULE_PUBLIC_API(NameExprRule);
};

// [[ParserDoc_PrimitiveExpr]]
class PrimitiveExprRule : public ParserBaseRule {
   public:
    RULE_PUBLIC_API(PrimitiveExprRule);
};

}  // namespace tm_parse::rules