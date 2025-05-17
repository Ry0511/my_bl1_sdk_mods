//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "text_mod_common.h"

namespace bl1_text_mods {

using token_int = int;

// clang-format off

enum class TokenKind : token_int {
    Kw_Set = 0       , // Set keyword
    Kw_None          , // None keyword
    Kw_Level         , // level keyword
    LeftParen        , // (
    RightParen       , // )
    Dot              , // .
    Colon            , // :
    Slash            , // /
    Star             , // *
    Comma            , // ,
    LeftBracket      , // [
    RightBracket     , // ]
    Number           , // [0-9]+ ( DOT [0-9]+ )?
    Equal            , // =
    Identifier       , // a-zA-Z0-9
    StringLiteral    , // ".*?"
    NameLiteral      , // '.*?'
    LineComment      , // # ...
    MultiLineComment , // /* ... */
    END_OF_INPUT     , // EOF
};

constexpr token_int begin_kw_token = static_cast<token_int>(TokenKind::Kw_Set);
constexpr token_int end_kw_token = static_cast<token_int>(TokenKind::Kw_Level);

constexpr str_view token_kind_names[]{
    TXT("set")             , // set keyword
    TXT("None")            , // None keyword
    TXT("level")           , // level keyword
    TXT("LeftParen")       , // (
    TXT("RightParen")      , // )
    TXT("Dot")             , // .
    TXT("Colon")           , // :
    TXT("Slash")           , // /
    TXT("Star")            , // *
    TXT("Comma")           , // ,
    TXT("LeftBracket")     , // [
    TXT("RightBracket")    , // ]
    TXT("Number")          , // [0-9]+ ( DOT [0-9]+ )?
    TXT("Equal")           , // =
    TXT("Identifier")      , // a-zA-Z0-9
    TXT("StringLiteral")   , // ".*?"
    TXT("NameLiteral")     , // '.*?'
    TXT("LineComment")     , // # ...
    TXT("MultiLineComment"), // /* ... */
    TXT("END_OF_INPUT")    , // EOF
};

// clang-format on

////////////////////////////////////////////////////////////////////////////////
// | TOKEN STRUCTURE |
////////////////////////////////////////////////////////////////////////////////

struct Token {
    TokenKind Kind{};
    str_view Text{};

    explicit constexpr Token() noexcept : Kind(TokenKind::END_OF_INPUT), Text() {};
    explicit constexpr Token(TokenKind kind, str_view text) noexcept : Kind(kind), Text(text) {};

    [[nodiscard]] std::string token_as_str() const noexcept {
        auto index = static_cast<std::size_t>(Kind);
        str_view token_kind_name = token_kind_names[index];
        return std::format("{:<17} = '{}'", str{token_kind_name}, str{Text});
    }

    // Text Compare
    bool operator==(const str_view& other) const noexcept { return this->Text == other; }
    bool operator!=(const str_view& other) const noexcept { return this->Text != other; }

    // Token Kind Compare
    bool operator==(const TokenKind& other) const noexcept { return this->Kind == other; }
    bool operator!=(const TokenKind& other) const noexcept { return this->Kind != other; }

    // Token Compare
    bool operator==(const Token& other) const noexcept { return this->Kind == other.Kind; }
    bool operator!=(const Token& other) const noexcept { return this->Kind != other.Kind; }
};

static constexpr Token TOK_EOF{TokenKind::END_OF_INPUT, str_view{}};

}  // namespace bl1_text_mods