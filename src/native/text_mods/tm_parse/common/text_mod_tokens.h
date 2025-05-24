//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "common/text_mod_common.h"

namespace tm_parse {
// clang-format off

using token_int = uint8_t;

// Note that keywords are case-insensitive
enum class TokenKind : token_int {
    Kw_Set = 0       , // Set keyword
    Kw_None          , // None keyword
    Kw_Level         , // level keyword
    Kw_Begin         , // Begin keyword
    Kw_Object        , // Object keyword
    Kw_Class         , // Class keyword
    Kw_Name          , // Name keyword
    Kw_Package       , // Package keyword
    Kw_End           , // End keyword
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
    Identifier       , // [a-zA-Z][\w\d_]+
    StringLiteral    , // ".*?"
    NameLiteral      , // '.*?'
    LineComment      , // # ...
    MultiLineComment , // /* ... */
    DelegateLine     , // __OnSkillGradeChanged__Delegate=(null).None (full line ignored)
    BlankLine        , // \n[\n\r\t ]+
    EndOfInput       , // EOF
    TokenKind_Count  , // Keep this last
};

constexpr token_int begin_kw_token     = static_cast<token_int>(TokenKind::Kw_Set);
constexpr token_int end_kw_token       = static_cast<token_int>(TokenKind::Kw_Count);
constexpr token_int begin_symbol_token = static_cast<token_int>(TokenKind::LeftParen);
constexpr size_t    token_count        = static_cast<size_t>(TokenKind::TokenKind_Count);

constexpr std::array<str_view, token_count> token_kind_names{
    TXT("Set"),               // Set keyword
    TXT("None"),              // None keyword
    TXT("Level"),             // level keyword
    TXT("Begin"),             // Begin keyword
    TXT("Object"),            // Object keyword
    TXT("Class"),             // Class keyword
    TXT("Name"),              // Name keyword
    TXT("Package"),           // Package keyword
    TXT("End"),               // End keyword
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
    TXT("DelegateLine"),      // __OnSkillGradeChanged__Delegate=(null).None (full line ignored)
    TXT("BlankLine"),         // \n+
    TXT("EndOfInput"),        // EOF
};

////////////////////////////////////////////////////////////////////////////////
// | ITERATORS |
////////////////////////////////////////////////////////////////////////////////

// lmao couln't help myself this shit is so overcomplicated.

struct TokenProxy {
    token_int Index;

    // Ctor
    constexpr TokenProxy(token_int kind) noexcept : Index(kind) {};

    // Getters
    constexpr token_int as_int() const noexcept     { return Index;                         }
    constexpr TokenKind as_token() const noexcept   { return static_cast<TokenKind>(Index); }
    constexpr str_view as_str_view() const noexcept { return token_kind_names.at(as_int()); }
    constexpr str as_str() const noexcept           { return str{as_str_view()};            }

    // Conversions
    constexpr operator token_int() const noexcept { return as_int();                      }
    constexpr operator TokenKind() const noexcept { return as_token();                    }
    constexpr operator str_view() const noexcept  { return token_kind_names.at(as_int()); }
};

template<token_int Begin, token_int End>
struct TokenRangeIterator {

    constexpr static token_int BeginValue = Begin;
    constexpr static token_int EndValue   = End;

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
    [[nodiscard]] bool is_identifier() const noexcept {
        const auto index = static_cast<token_int>(Kind);
        return Kind == TokenKind::Identifier || begin_kw_token <= index && index < end_kw_token;
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

static constexpr Token token_eof{TokenKind::EndOfInput, str_view{}};

}  // namespace tm_parse