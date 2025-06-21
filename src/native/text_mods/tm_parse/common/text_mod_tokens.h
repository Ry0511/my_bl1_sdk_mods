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
    Kw_Invalid       , // Invalid; Used exclusively in (INVALID) enum checks
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
    Identifier       , // [a-zA-Z_][\w\d_]+
    StringLiteral    , // ".*?"
    NameLiteral      , // '.*?'
    LineComment      , // # ...
    MultiLineComment , // /* ... */
    BlankLine        , // \n[\n\r\t ]+
    EndOfInput       , // EOF
    TokenKind_Count  , // Keep this last
};

namespace tokens {
// NOLINTBEGIN(*-unused-using-decls)
using TokenKind::Kw_Set;
using TokenKind::Kw_None;
using TokenKind::Kw_Level;
using TokenKind::Kw_Begin;
using TokenKind::Kw_Object;
using TokenKind::Kw_Class;
using TokenKind::Kw_Name;
using TokenKind::Kw_Package;
using TokenKind::Kw_End;
using TokenKind::Kw_True;
using TokenKind::Kw_False;
using TokenKind::Kw_Invalid;
using TokenKind::Kw_Count;
using TokenKind::LeftParen;
using TokenKind::RightParen;
using TokenKind::Dot;
using TokenKind::Colon;
using TokenKind::Slash;
using TokenKind::Star;
using TokenKind::Comma;
using TokenKind::LeftBracket;
using TokenKind::RightBracket;
using TokenKind::Number;
using TokenKind::Equal;
using TokenKind::Identifier;
using TokenKind::StringLiteral;
using TokenKind::NameLiteral;
using TokenKind::LineComment;
using TokenKind::MultiLineComment;
using TokenKind::BlankLine;
using TokenKind::EndOfInput;
using TokenKind::TokenKind_Count;
// NOLINTEND(*-unused-using-decls)
}

constexpr token_int begin_kw_token     = static_cast<token_int>(TokenKind::Kw_Set);
constexpr token_int end_kw_token       = static_cast<token_int>(TokenKind::Kw_Count);
constexpr token_int begin_symbol_token = static_cast<token_int>(TokenKind::LeftParen);
constexpr size_t    token_count        = static_cast<size_t>(TokenKind::TokenKind_Count);

static_assert(begin_kw_token == 0, "Starting keyword should be Zero");

constexpr std::array<str_view, token_count + 1> token_kind_names{
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
    TXT("Invalid"),           // Invalid; Used exclusively in (INVALID) enum checks
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
    TXT("_INVALID_"),         // Invalid Token
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
        constexpr bool operator!=(const Iterator& other) const noexcept { return Index != other.Index; }
        constexpr TokenProxy operator*() const noexcept { return TokenProxy{Index}; }
        constexpr Iterator& operator++() noexcept       { ++Index; return *this; }
    };

public:
    constexpr TokenRangeIterator() noexcept = default;
    constexpr auto begin() const noexcept { return Iterator{Begin}; }
    constexpr auto end() const noexcept   { return Iterator{End};   }
};

using KeywordTokenIterator = TokenRangeIterator<begin_kw_token, end_kw_token>;
using SymbolTokenIterator  = TokenRangeIterator<begin_symbol_token, static_cast<token_int>(token_count)>;

consteval size_t largest_token_len() noexcept {
    size_t largest_token_len = 0;
    for (auto proxy : KeywordTokenIterator{}) {
        largest_token_len = std::max(largest_token_len, proxy.as_str_view().size());
    }
    return largest_token_len;
}

consteval size_t smallest_token_len() noexcept {
    size_t smallest_token_len = std::numeric_limits<size_t>::max();
    for (auto proxy : KeywordTokenIterator{}) {
        smallest_token_len = std::min(smallest_token_len, proxy.as_str_view().size());
    }
    return smallest_token_len;
}

constexpr size_t min_token_len = smallest_token_len();
constexpr size_t max_token_len = largest_token_len();

// clang-format on

////////////////////////////////////////////////////////////////////////////////
// | TOKEN STRUCTURE |
////////////////////////////////////////////////////////////////////////////////

struct TokenTextView {
    size_t Start;
    size_t Length;

    constexpr TokenTextView() noexcept : Start(invalid_index_v), Length(invalid_index_v) {};
    constexpr TokenTextView(size_t start, size_t len) noexcept : Start(start), Length(len) {};

    str_view view_from(str_view in_vw) const noexcept { return in_vw.substr(Start, Length); }

    constexpr bool operator==(const TokenTextView& other) const noexcept = default;
    constexpr bool operator!=(const TokenTextView& other) const noexcept = default;

    constexpr bool is_valid() const noexcept { return Start != invalid_index_v && Length != invalid_index_v; };
    [[nodiscard]] size_t end() const noexcept { return Start + Length; };

    /**
     * Extends this view to encompass the provided and this current view.
     * @param other The other view to encompass.
     * @throws std::runtime_error if `this` or `other` is `!is_valid()`
     */
    void extend(const TokenTextView& other) {
        if (!is_valid() || !other.is_valid()) {
            throw std::runtime_error("Cannot extend invalid token view");
        }

        // - Extend the view to encompass both views
        // A = (5,  3)
        // B = (9,  6)
        // C = (5, 10); max(5 + 3, 9 + 6) - 5

        Start = std::min(Start, other.Start);
        Length = std::max(end(), other.end()) - Start;
    }
};

struct Token {
    TokenKind Kind;
    str_view SourceStr;
    TokenTextView TextRegion;

    constexpr Token() noexcept : Kind(TokenKind::EndOfInput) {};
    explicit constexpr Token(TokenKind kind, TokenTextView text) noexcept : Kind(kind), TextRegion(text) {};
    constexpr ~Token() noexcept = default;

    constexpr Token(const Token&) = default;
    constexpr Token& operator=(const Token&) = default;
    constexpr Token(Token&&) = default;
    constexpr Token& operator=(Token&&) = default;

    [[nodiscard]] std::string token_as_str() const noexcept {
        str token_kind_name = TokenProxy{Kind};

        // This is just a garbage token really
        if (Kind == TokenKind::BlankLine) {
            return std::format("{:<17} = {}", token_kind_name, str{TXT("_BLANK_")});
        }

        return std::format("{:<17} = {}", token_kind_name, as_str());
    }

    // clang-format off
    str_view as_str_view()      const noexcept { return TextRegion.view_from(SourceStr);      }
    str      as_str()           const noexcept { return str{TextRegion.view_from(SourceStr)}; }
    str_view kind_as_str_view() const noexcept { return TokenProxy{Kind}.as_str_view();       };
    str      kind_as_str()      const noexcept { return TokenProxy{Kind}.as_str();            };
    // clang-format on

    str to_string() const noexcept {
        switch (Kind) {
            case TokenKind::BlankLine:
                return TXT("_BLANK_");
            case TokenKind::EndOfInput:
                return TXT("_EOF_");
            case TokenKind::TokenKind_Count:
                return TXT("_INVALID_");
            default:
                return as_str();
        }
    }

    /**
     * @return True if this token is an Identifier token or a keyword token
     */
    [[nodiscard]] bool is_identifier() const noexcept { return Kind == TokenKind::Identifier || is_keyword(); }

    [[nodiscard]] bool is_keyword() const noexcept { return static_cast<token_int>(Kind) < end_kw_token; }

    [[nodiscard]] bool is_comment() const noexcept {
        constexpr std::array<TokenKind, 2> skip_tokens = {
            TokenKind::LineComment,
            TokenKind::MultiLineComment,
        };

        return std::ranges::find(skip_tokens, Kind) != skip_tokens.end();
    }

    [[nodiscard]] bool is_any(std::span<TokenKind> kinds) const noexcept {
        return std::ranges::find(kinds, Kind) != kinds.end();
    }

    /**
     * @return True if this token is a blank line or the end of input.
     */
    [[nodiscard]] bool is_eolf() const noexcept { return is_any<TokenKind::EndOfInput, TokenKind::BlankLine>(); }

    template <TokenKind... Kinds>
    [[nodiscard]] bool is_any() const noexcept {
        return (... || (Kind == Kinds));
    }

    // Token Kind Compare
    bool operator==(const TokenKind& other) const noexcept { return this->Kind == other; }
    bool operator!=(const TokenKind& other) const noexcept { return this->Kind != other; }

    // Token Compare
    bool operator==(const Token& other) const noexcept { return this->Kind == other.Kind; }
    bool operator!=(const Token& other) const noexcept { return this->Kind != other.Kind; }
};

static constexpr Token token_eof{TokenKind::EndOfInput, TokenTextView{}};
static constexpr Token token_invalid{TokenKind::TokenKind_Count, TokenTextView{}};

}  // namespace tm_parse