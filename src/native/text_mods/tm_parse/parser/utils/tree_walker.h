//
// Date       : 11/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"
#include "parser/rules/primary_rules.h"

namespace tm_parse::utils {

#define WALKER(type) \
    template <>      \
    void walk(const rules::type& rule, const auto& func) const noexcept

class TreeWalker {
   public:
    template <class T>
    void walk(const T& rule, const auto& func) const noexcept = delete;

   public:
    WALKER(ProgramRule);
    WALKER(ObjectDefinitionRule);
    WALKER(SetCommandRule);

    WALKER(IdentifierRule);
    WALKER(DotIdentifierRule);
    WALKER(ObjectIdentifierRule);
    WALKER(ObjectAccessRule);
    WALKER(ArrayAccessRule);
    WALKER(PropertyAccessRule);

    WALKER(NumberExprRule);
    WALKER(StrExprRule);
    WALKER(NameExprRule);
    WALKER(KeywordRule);
    WALKER(LiteralExprRule);

    WALKER(AssignmentExprRule);
    WALKER(AssignmentExprListRule);
    WALKER(ParenExprRule);
    WALKER(ExpressionRule);
};

#undef WALKER

}  // namespace tm_parse::utils
