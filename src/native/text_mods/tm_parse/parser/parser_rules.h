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

////////////////////////////////////////////////////////////////////////////////
// | IDENTIFIERS |
////////////////////////////////////////////////////////////////////////////////

// [[ParserDoc_DotIdentifier]]
class DotIdentifierRule : public ParserBaseRule {
   private:
    size_t m_StartTokenIndex{invalid_index_v};  // Index to start token
    size_t m_EndTokenIndex{invalid_index_v};    // Index to end token

   public:
    [[nodiscard]] size_t start_token_index() const { return m_StartTokenIndex; }
    [[nodiscard]] size_t end_token_index() const { return m_EndTokenIndex; }

    RULE_PUBLIC_API(DotIdentifierRule);
};

// [[ParserDoc_ObjectIdentifier]]
class ObjectIdentifierRule : public ParserBaseRule {
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
    // Note: we only need the first index since
    //  (S + 1 == Number) and (S + 2 == Closing Token)
   private:
    size_t m_StartTokenIndex{invalid_index_v};  // Index to start token ( or [

   public:
    [[nodiscard]] size_t start_token_index() const { return m_StartTokenIndex; }

    RULE_PUBLIC_API(ArrayAccessRule);
};

// [[ParserDoc_PropertyAccess]]
class PropertyAccessRule : public ParserBaseRule {
   private:
    size_t m_StartTokenIndex{invalid_index_v};  // Index to identifier token
    ArrayAccessRule m_ArrayAccess;              // optional

   public:
    [[nodiscard]] size_t start_token_index() const { return m_StartTokenIndex; }
    [[nodiscard]] const ArrayAccessRule& array_access() const { return m_ArrayAccess; }

   public:
    RULE_PUBLIC_API(PropertyAccessRule);
};

////////////////////////////////////////////////////////////////////////////////
// | EXPRESSIONS |
////////////////////////////////////////////////////////////////////////////////

// [[ParserDoc_ParenExpr]]
class ParenExprRule : public ParserBaseRule {
   private:
    size_t m_StartTokenIndex{invalid_index_v};  // Index to Left Paren
    size_t m_EndTokenIndex{invalid_index_v};    // Index to Right Paren

   public:
    size_t start_token_index() const noexcept(true) { return m_StartTokenIndex; }
    size_t end_token_index() const noexcept(true) { return m_EndTokenIndex; }

   public:
    RULE_PUBLIC_API(ParenExprRule);
};

// [[ParserDoc_CompositeExpr]]
class CompositeExprRule : public ParserBaseRule {
   public:
    using InvalidTag = std::monostate;
    using ExprType = std::variant<InvalidTag, PrimitiveExprRule, ParenExprRule>;

   private:
    ExprType m_Expr;  // The value expression

   public:
    template <class T>
    bool has_value() const noexcept(true) {
        return std::holds_alternative<T>(m_Expr);
    }

   public:
    RULE_PUBLIC_API(CompositeExprRule);
};

////////////////////////////////////////////////////////////////////////////////
// | PRIMARY NODES |
////////////////////////////////////////////////////////////////////////////////

// [[ParserDoc_SetCommand]]
class SetCommandRule : public ParserPrimaryRule {
   private:
    size_t m_StartTokenIndex{invalid_index_v};  // Index to KW_Set token
    ObjectIdentifierRule m_ObjectIdentifier;    // Source object
    PropertyAccessRule m_PropertyAccess;        // Property to set
    CompositeExprRule m_ValueExpr;              // Value expression

   public:
    [[nodiscard]] size_t start_token_index() const { return m_StartTokenIndex; }
    [[nodiscard]] const ObjectIdentifierRule& object_identifier() const { return m_ObjectIdentifier; }
    [[nodiscard]] const PropertyAccessRule& property_access() const { return m_PropertyAccess; }
    [[nodiscard]] const CompositeExprRule& value_expr() const { return m_ValueExpr; }

    RULE_PUBLIC_API(SetCommandRule);
};

}  // namespace tm_parse::rules
