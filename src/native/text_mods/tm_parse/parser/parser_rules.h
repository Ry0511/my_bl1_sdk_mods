//
// Date       : 24/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "parser_rule_enum.h"

namespace tm_parse::rules {

// - NOTE -
// The lexer contains skip tokens ( BlankLine ) which is unfortunately required to handle the
//  following object definition. I don't want to modify the input source if possible since the idea
//  is that users will copy directly from the editor or an external tool.
//
// ```
//   Begin Object Class=SomeClass Name=SomeName
//     Foo=
//     Bar=(
//       A=,
//       B=2
//     )
//   End Object
// ```
//
// In this case the token generation for Foo is Identifier Equal BlankLine Identifier ...
//  this makes it easy to identify where Foo ends and Bar begins. But means most new line have their
//  own token in the stream which is far from ideal. This burden is reduced slightly with how the
//  lexer handles BlankLines consuming via \n[\r\n\t ]*
//
// Example token generation:
//  | Token       | As String                                    | Comment
//  |-------------|------------------------------------------------------------
//  | Identifier  | Outer                                        |
//  | Equal       | =                                            |
//  | Level       | Level                                        | Picked up as a Keyword but is an identifier
//  | NameLiteral | 'menumap.TheWorld:PersistentLevel'           |
//  | BlankLine   | _BLANK_                                      |
//  | Name        | Name                                         |
//  | Equal       | =                                            |
//  | Identifier  | WillowPlayerController_0                     | Assigning to an identifier (see below)
//  | BlankLine   | _BLANK_                                      |
//  | Class       | Class                                        |
//  | Equal       | =                                            |
//  | Class       | Class                                        |
//  | NameLiteral | 'WillowGame.WillowPlayerController'          |
//  | BlankLine   | _BLANK_                                      |
//  | Identifier  | ObjectArchetype                              |
//  | Equal       | =                                            |
//  | Identifier  | WillowPlayerController                       |
//  | NameLiteral | 'WillowGame.Default__WillowPlayerController' |
//  | BlankLine   | _BLANK_                                      |
//  | End         | End                                          |
//  | Object      | Object                                       |
//
// Set commands don't really run into this issue but they do run into a different issue with
//  unquoted strings.
//
// i.e.,
//
// ```
//   set SomeClass SomeProperty Some Text
//   set SomeClass SomeProperty ()
// ```
//
// In the case above there are a few ways it can be handled. You can consume the token stream until
// you reach a Set, Begin, or EOF token (effectively the start of a new token or the end of the file).
// The alternative, which is what is implemented is to read upto the first BlankLine token. Both
// approaches should work and the former seems like a better choice however because we also want to
// support object definitions we have no choice but to include the BlankLine token anyway.
//
//

}  // namespace tm_parse::rules
