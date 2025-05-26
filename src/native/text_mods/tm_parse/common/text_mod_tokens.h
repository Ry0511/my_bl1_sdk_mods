//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include <algorithm>

#include "common/text_mod_common.h"

namespace tm_parse {
// clang-format off

using token_int = uint8_t;

// Note that keywords are case-insensitive
enum class TokenKind : token_int {
    Kw_Set = 0       , // Set
    Kw_None          , // None
    Kw_Level         , // level
    Kw_Begin         , // Begin
    Kw_Object        , // Object
    Kw_Class         , // Class
    Kw_Name          , // Name
    Kw_Package       , // Package
    Kw_End           , // End
    Kw_True          , // True
    Kw_False         , // False
    Kw_Count         , // Count of keywords
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
    Identifier       , // [a-zA-Z][\w\d_]+ //TODO: Can start with underscores...
    StringLiteral    , // ".*?"
    NameLiteral      , // '.*?'
    LineComment      , // # ...
    MultiLineComment , // /* ... */
    BlankLine        , // \n[\n\r\t ]+
    EndOfInput       , // EOF
    TokenKind_Count  , // Keep this last
};

constexpr token_int begin_kw_token     = static_cast<token_int>(TokenKind::Kw_Set);
constexpr token_int end_kw_token       = static_cast<token_int>(TokenKind::Kw_Count);
constexpr token_int begin_symbol_token = static_cast<token_int>(TokenKind::LeftParen);
constexpr size_t    token_count        = static_cast<size_t>(TokenKind::TokenKind_Count);

static_assert(begin_kw_token == 0, "Starting keyword should be Zero");

constexpr std::array<str_view, token_count> token_kind_names{
    TXT("Set"),               // Set
    TXT("None"),              // None
    TXT("Level"),             // level
    TXT("Begin"),             // Begin
    TXT("Object"),            // Object
    TXT("Class"),             // Class
    TXT("Name"),              // Name
    TXT("Package"),           // Package
    TXT("End"),               // End
    TXT("True"),              // True
    TXT("False"),             // False
    TXT("Count"),             // Count of keywords
    TXT("LeftParen"),         // (
    TXT("RightParen"),        // )
    TXT("Dot"),               // .
    TXT("Colon"),             // :
    TXT("Slash"),             // /
    TXT("Star"),              // *
    TXT("Comma"),             // ,
    TXT("LeftBracket"),       // [
    TXT("RightBracket"),      // ]
    TXT("Number"),            // [0-9]+ ( DOT [0-9]+ )?
    TXT("Equal"),             // =
    TXT("Identifier"),        // a-zA-Z0-9
    TXT("StringLiteral"),     // ".*?"
    TXT("NameLiteral"),       // '.*?'
    TXT("LineComment"),       // # ...
    TXT("MultiLineComment"),  // /* ... */
    TXT("BlankLine"),         // \n+
    TXT("EndOfInput"),        // EOF
};

////////////////////////////////////////////////////////////////////////////////
// | ITERATORS |
////////////////////////////////////////////////////////////////////////////////

// Why?..

struct TokenProxy {
    token_int Index;

    // Ctor
    constexpr TokenProxy(token_int index) noexcept : Index(index) {};
    constexpr TokenProxy(TokenKind kind) noexcept : Index(static_cast<token_int>(kind)) {};

    // Getters
    [[nodiscard]] constexpr token_int as_int() const noexcept     { return Index;                         }
    [[nodiscard]] constexpr TokenKind as_token() const noexcept   { return static_cast<TokenKind>(Index); }
    [[nodiscard]] constexpr str_view as_str_view() const noexcept { return token_kind_names.at(as_int()); }
    [[nodiscard]] constexpr str as_str() const noexcept           { return str{as_str_view()};            }

    // Conversions
    constexpr operator token_int() const noexcept { return as_int();                      }
    constexpr operator TokenKind() const noexcept { return as_token();                    }
    constexpr operator str_view() const noexcept  { return token_kind_names.at(as_int()); }
    constexpr operator str() const noexcept       { return str{operator str_view()};      }
};

template<token_int Begin, token_int End>
struct TokenRangeIterator {

    constexpr static token_int begin_value = Begin;
    constexpr static token_int end_value   = End;

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = TokenProxy;
        using reference         = TokenProxy&;

    public:
        token_int Index;

    public:
        constexpr Iterator(token_int index) noexcept : Index(index) {};
        bool operator!=(const Iterator& other) const noexcept { return Index != other.Index; }
        TokenProxy operator*() const noexcept { return TokenProxy{Index}; }
        Iterator& operator++() noexcept       { ++Index; return *this; }
    };

public:
    constexpr TokenRangeIterator() noexcept = default;
    constexpr auto begin() const noexcept { return Iterator{Begin}; }
    constexpr auto end() const noexcept   { return Iterator{End};   }
};

using KeywordTokenIterator = TokenRangeIterator<begin_kw_token, end_kw_token>;
using SymbolTokenIterator  = TokenRangeIterator<begin_symbol_token, static_cast<token_int>(token_count)>;

// clang-format on

////////////////////////////////////////////////////////////////////////////////
// | TOKEN STRUCTURE |
////////////////////////////////////////////////////////////////////////////////

struct Token {
    TokenKind Kind;
    str_view Text;

    explicit constexpr Token() noexcept : Kind(TokenKind::EndOfInput) {};
    explicit constexpr Token(TokenKind kind, str_view text) noexcept : Kind(kind), Text(text) {};

    [[nodiscard]] std::string token_as_str() const noexcept {
        const auto index = static_cast<token_int>(Kind);
        const str_view token_kind_name = token_kind_names.at(static_cast<size_t>(index));

        // This is just a garbage token really
        if (Kind == TokenKind::BlankLine) {
            return std::format("{:<17} = {}", str{token_kind_name}, str{TXT("_BLANK_")});
        }

        return std::format("{:<17} = {}", str{token_kind_name}, str{Text});
    }

    /**
     * @return True if this token is an Identifier token or a keyword token
     */
    [[nodiscard]] bool is_identifier() const noexcept { return Kind == TokenKind::Identifier || is_keyword(); }

    [[nodiscard]] bool is_keyword() const noexcept { return static_cast<token_int>(Kind) < end_kw_token; }

    [[nodiscard]] bool is_skip_token() const noexcept {
        constexpr std::array<TokenKind, 4> skip_tokens = {
            TokenKind::BlankLine,
            TokenKind::LineComment,
            TokenKind::MultiLineComment,
        };

        return std::ranges::find(skip_tokens, Kind) != skip_tokens.end();
    }

    [[nodiscard]] bool is_any(std::span<TokenKind> kinds) const noexcept {
        return std::ranges::find(kinds, Kind) != kinds.end();
    }

    // Text Compare
    bool operator==(str_view other) const noexcept { return this->Text == other; }
    bool operator!=(str_view other) const noexcept { return this->Text != other; }

    // Token Kind Compare
    bool operator==(const TokenKind& other) const noexcept { return this->Kind == other; }
    bool operator!=(const TokenKind& other) const noexcept { return this->Kind != other; }

    // Token Compare
    bool operator==(const Token& other) const noexcept { return this->Kind == other.Kind; }
    bool operator!=(const Token& other) const noexcept { return this->Kind != other.Kind; }
};

static constexpr Token token_eof{TokenKind::EndOfInput, str_view{}};

}  // namespace tm_parse