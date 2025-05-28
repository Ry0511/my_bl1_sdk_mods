//
// Date       : 25/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

namespace tm_parse {

// clang-format off

/**
================================================================================

Structural overview of the parser rules and how they are handled.
 - See Lexer Tokens: [[../common/text_mod_tokens.h]]
 - See WPC Example : [[../standalone/wpc_obj_dump_utf-8.txt]]

I have tried to avoid reusing names from the Lexer but somethings
 really don't change much from the lexer side except compositing a
 couple tokens.

TODO: Fix Identifier lexing to match: [a-zA-Z_][a-zA-Z_\d]*
      var(AdvancedSettings) bool _bUseAdvancedSettings;
      careful of: __CanUnpause__Delegate=(null).None

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
    - TODO: Currently doesn't parse the expression just consumes parens
  - RelativeRotation=(Pitch=0,Yaw=0,Roll=0)

- #[[ParserDoc_CompositeExpr]]
  - [[PrimitiveExpr]] | [[ParenExpr]]

================================================================================
== Primary Nodes ===============================================================

- #[[ParserDoc_SetCommand]]
  - Kw_Set ObjectIdentifier Identifier CompositeExpr

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

}  // namespace tm_parse