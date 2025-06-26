//
// Date       : 24/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "parser/parser_rule_enum.h"
#include "parser/primitive_expr_rules.h"

namespace tm_parse {
class TextModLexer;
class TextModParser;
};  // namespace tm_parse

namespace tm_parse::rules {

class IdentifierRule : public ParserBaseRule {
   public:
    RULE_PUBLIC_API(IdentifierRule);
};

// [[ParserDoc_DotIdentifier]]
class DotIdentifierRule : public ParserBaseRule {
   public:
    RULE_PUBLIC_API(DotIdentifierRule);
};

// [[ParserDoc_ObjectIdentifier]]
class ObjectIdentifierRule : public ParserBaseRule {
    // Originally assumed this was at most one child identifier but now that I think of it there
    //  can be any number of children here. i.e., A:B:C:D:E which does complicate things slightly.
    private:
    DotIdentifierRule m_PrimaryIdentifier;  // The primary identifier (guaranteed)
    DotIdentifierRule m_ChildIdentifier;    // The child identifier (optional)

   public:
    [[nodiscard]] const DotIdentifierRule& primary_identifier() const { return m_PrimaryIdentifier; }
    [[nodiscard]] const DotIdentifierRule& child_identifier() const { return m_ChildIdentifier; }

    RULE_PUBLIC_API(ObjectIdentifierRule);
};

// [[ParserDoc_ArrayAccess]]
class ArrayAccessRule : public ParserBaseRule {
   private:
    NumberExprRule m_Index;

   public:
    RULE_PUBLIC_API(ArrayAccessRule);
};

// [[ParserDoc_PropertyAccess]]
class PropertyAccessRule : public ParserBaseRule {
   private:
    IdentifierRule m_Identifier;
    ArrayAccessRule m_ArrayAccess;

   public:
    [[nodiscard]] const IdentifierRule& identifier() const { return m_Identifier; }
    [[nodiscard]] const ArrayAccessRule& array_access() const { return m_ArrayAccess; }

   public:
    RULE_PUBLIC_API(PropertyAccessRule);
};

}  // namespace tm_parse::rules
