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

// - NOTE -
// All parser rules are defined by indices into a parser lookup table.
//

////////////////////////////////////////////////////////////////////////////////
// | IDENTIFIERS |
////////////////////////////////////////////////////////////////////////////////

// [[ParserDoc_DotIdentifier]]
class DotIdentifierRule {
   private:
    size_t m_StartIndex = invalid_index_v;  // Inclusive Start
    size_t m_EndIndex = invalid_index_v;    // Inclusive End

   public:
    [[nodiscard]] size_t start_index() const { return m_StartIndex; }
    [[nodiscard]] size_t end_index() const { return m_EndIndex; }

   public:
    str_view to_string(const TextModParser& parser) const noexcept;
    operator bool() const noexcept(true);

   public:
    static DotIdentifierRule create(TextModParser* parser);
};

// [[ParserDoc_ObjectIdentifier]]
class ObjectIdentifierRule {
   private:
    DotIdentifierRule m_PrimaryIdentifier{};
    DotIdentifierRule m_SecondaryIdentifier{};

   public:
    [[nodiscard]] const DotIdentifierRule& primary_identifier() const { return m_PrimaryIdentifier; }
    [[nodiscard]] const DotIdentifierRule& secondary_identifier() const { return m_SecondaryIdentifier; }

   public:
    str_view to_string(const TextModParser& parser) const noexcept;

   public:
    static ObjectIdentifierRule create(TextModParser* parser);
};

// [[ParserDoc_ArrayAccess]]
class ArrayAccessRule {
   private:
    size_t m_OpenTokenIndex = invalid_index_v;

   public:
    [[nodiscard]] size_t open_token_index() const { return m_OpenTokenIndex; }
    [[nodiscard]] size_t number_token_index() const noexcept { return m_OpenTokenIndex + 1; }
    [[nodiscard]] size_t close_token_index() const noexcept { return m_OpenTokenIndex + 2; }

   public:
    str_view to_string(const TextModParser& parser) const noexcept;

   public:
    operator bool() const noexcept(true) { return open_token_index() != invalid_index_v; }
    static ArrayAccessRule create(TextModParser* parser);
};

struct PropertyAccessRule {
   private:
    size_t IdentifierTokenIndex = invalid_index_v;
    ArrayAccessRule ArrayAccess{};

   public:
    str_view to_string(const TextModParser& parser) const noexcept;

   public:
    [[nodiscard]] size_t identifier_token_index() const { return IdentifierTokenIndex; }
    [[nodiscard]] const ArrayAccessRule& array_access() const { return ArrayAccess; }

   public:
    operator bool() const noexcept(true) { return IdentifierTokenIndex != invalid_index_v; }
    static PropertyAccessRule create(TextModParser* parser);
};

////////////////////////////////////////////////////////////////////////////////
// | VALUE EXPRESSIONS |
////////////////////////////////////////////////////////////////////////////////

struct CompositeExprRule {};

////////////////////////////////////////////////////////////////////////////////
// | ROOT NODES |
////////////////////////////////////////////////////////////////////////////////

// [[ParserDoc_SetCommand]]
class SetCommandRule {
   private:
    size_t m_SetCommandIndex = invalid_index_v;
    ObjectIdentifierRule m_ObjectIdentifier{};
    PropertyAccessRule m_PropertyAccess{};
    CompositeExprRule m_CompositeExpr{};

   public:
    str_view to_string(const TextModParser& parser) const noexcept;

   public:
    static SetCommandRule create(TextModParser* parser);
};

struct ProgramRule {};

}  // namespace tm_parse::rules
