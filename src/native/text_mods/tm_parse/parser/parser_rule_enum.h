//
// Date       : 25/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

namespace tm_parse {

// clang-format off

// TODO: Revisit this documentation, likely good to remove it and keep the actual definitions in the markdowns docs.

/**
================================================================================

Structural overview of the parser rules and how they are handled.
 - See Lexer Tokens: [[../common/text_mod_tokens.h]]
 - See WPC Example : [[../standalone/wpc_obj_dump_utf-8.txt]]

I have tried to avoid reusing names from the Lexer but somethings
 really don't change much from the lexer side except compositing a
 couple tokens.

================================================================================
== Identifiers =================================================================

- #[[ParserDoc_DotIdentifier]]
  - Identifier ( Dot Identifier )*
  - menumap.TheWorld

- #[[ParserDoc_ObjectIdentifier]]
  - DotIdentifier (Colon DotIdentifier)?
  - menumap.TheWorld:PersistentLevel.WillowPlayerController_0.WillowAutoAimStrategy_0

- #[[ParserDoc_ArrayAccess]]
  - (LeftParen Number RightParen) | (LeftBracket Number RightBracket)
  - Foo(0) Baz[0]
  - () is for dynamic arrays and [] is for static/stack arrays

- #[[ParserDoc_PropertyAccess]]
  - Identifier ArrayAccess?
  - Foo(0) or Foo[0] or Foo

- #[[ParserDoc_PropertyAssignment]]
  - PropertyAccess Equal CompositeExpr
  - AmmoResourceUpgrades[7]=0
    CurrentTouchedPickupable=None

================================================================================
== Value Expressions ===========================================================

- #[[ParserDoc_ObjectNameLiteral]]
  - Identifier NameLiteral
  - WillowAutoAimStrategy'menumap.TheWorld:PersistentLevel.WillowPlayerController_0.WillowAutoAimStrategy_0'

- #[[ParserDoc_IdentifierChain]]
  - Identifier+ BlankLine
  - QuickSaveString=Quick Saving

- #[[ParserDoc_PrimitiveExpr]]
  - Number | StringLiteral | NameLiteralRule | IdentifierChainRule
    - StringLiteral might only be used in [[ParserDoc_ArrayAccess]]
  - ProgressTimeOut=8.000000
    QuickSaveString=Quick Saving
    NoPauseMessage=Game is not pauseable

- #[[ParserDoc_ParenExpr]]
  - LeftParen * RightParen
  - RelativeRotation=(Pitch=0,Yaw=0,Roll=0)

- #[[ParserDoc_CompositeExpr]]
  - [[PrimitiveExpr]] | [[ParenExpr]]

================================================================================
== Primary Nodes ===============================================================

- #[[ParserDoc_SetCommand]]
  - Kw_Set ObjectIdentifier PropertyAccess CompositeExpr

- #[[ParserDoc_ObjectDefinition]]
  - Kw_Begin Kw_Object
      Kw_Class Equal DotIdentifier
      Kw_Name Equal DotIdentifier

*/

// clang-format on

using parser_rule_int = uint8_t;

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
   public:                                      \
    constexpr type() noexcept(true) = default;  \
    constexpr ~type() noexcept(true) = default; \
    static type create(TextModParser&)

}  // namespace rules
}  // namespace tm_parse