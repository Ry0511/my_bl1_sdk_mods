//
// Date       : 25/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

namespace tm_parse {

using parser_rule_int = uint8_t;

// TODO: Refactor
// clang-format off
enum class ParserRuleKind : parser_rule_int {
    DotIdentifier     ,
    ObjectIdentifier  ,
    ArrayAccess       ,
    PropertyAccess    ,
    PropertyAssignment,
    AssignmentExpr    ,
    AssignmentExprList,
    ObjectNameLiteral ,
    IdentifierChain   ,
    PrimitiveExpr     ,
    ParenExpr         ,
    CompositeExpr     ,
    SetCommand        , // Primary Node
    ObjectDefinition  , // Primary Node
    Unknown           , // Default/entry for parser
};
// clang-format on

////////////////////////////////////////////////////////////////////////////////
// | GENERIC RULE SETUP |
////////////////////////////////////////////////////////////////////////////////

class TextModLexer;
class TextModParser;

namespace rules {

// TODO: String wrapper for lazily promoting to internal buffer

class ParserBaseRule {
   protected:
    TokenTextView m_TextRegion;
    std::shared_ptr<str> m_Text;

   public:
    const TokenTextView& text_region() const noexcept(true) { return m_TextRegion; }
    operator bool() const noexcept(true) { return m_TextRegion.is_valid(); }

   public:
    str_view to_string(TextModParser& parser) const;

    str_view to_string() const noexcept {
        if (m_Text == nullptr) {
            return str_view{};
        }
        return *m_Text;
    }

    void copy_str_internal(TextModParser& parser);
    bool has_copy_str() const noexcept { return m_Text != nullptr; }
};

// Not sure if this is actually a good idea or even useful
class ParserPrimaryRule : public ParserBaseRule {
   protected:
    TextModParser* m_Parser{nullptr};
};

#define RULE_PUBLIC_API(type)                   \
   public:                                      \
    constexpr type() noexcept(true) = default;  \
    constexpr ~type() noexcept(true) = default; \
    static type create(TextModParser&)

}  // namespace rules
}  // namespace tm_parse