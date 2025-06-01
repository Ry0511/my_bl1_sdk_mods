//
// Date       : 24/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "parser_rule_enum.h"

namespace tm_parse {
class TextModLexer;
class TextModParser;
};  // namespace tm_parse

namespace tm_parse::rules {

////////////////////////////////////////////////////////////////////////////////
// | GENERIC RULE SETUP |
////////////////////////////////////////////////////////////////////////////////

class ParserBaseRule {
   protected:
    TokenTextView m_TextRegion;

   public:
    const TokenTextView& text_region() const noexcept(true) { return m_TextRegion; }
    operator bool() const noexcept(true) { return m_TextRegion.is_valid(); }

   public:
    str_view to_string(TextModParser& parser) const;
};

// Not sure if this is actually a good idea or even useful
class ParserPrimaryRule : public ParserBaseRule {
   protected:
    TextModParser* m_Parser{nullptr};
};

#define RULE_PUBLIC_API(type)                   \
    constexpr type() noexcept(true) = default;  \
    constexpr ~type() noexcept(true) = default; \
    static type create(TextModParser&)

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

// - NOTE -
// This is anything on the right hand side of a full expression i.e.,
//   > set Foo.Baz:Bar Foo    Some Expression    | IdentifierChain
//   > set Foo.Baz:Bar Foo    (X=1,Y=2,Z=3,W=4)  | ParenExpr
//   > set Foo.Baz:Bar Foo    3.14               | Number
//   > set Foo.Baz:Bar Foo(0) "String Literal"   | StringLiteral
//   > set Foo.Baz:Bar Foo(0) Baz'Name.Literal'  | NameLiteral
//
// Some expressions are trivial types, that is, they are single decomposable values such as
// identifier chains, numbers, name literals, string literals, empty expressions, etc.
//

// [[ParserDoc_PrimitiveExpr]]
class PrimitiveExprRule : public ParserBaseRule {
    // TODO: Handle this for primitive types
   private:
    size_t m_StartTokenIndex{invalid_index_v};  // Index to start token

   public:
    size_t start_token_index() const noexcept(true) { return m_StartTokenIndex; }

   public:
    RULE_PUBLIC_API(PrimitiveExprRule);
};

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
