//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

////////////////////////////////////////////////////////////////////////////////
// | INCLUDES |
////////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "text_mod_utils.h"

////////////////////////////////////////////////////////////////////////////////
// | DEFINES |
////////////////////////////////////////////////////////////////////////////////

#define TXT_LOG(msg, ...) std::cout << std::format("[{}] " msg "\n", __FUNCTION__ __VA_OPT__(, ) __VA_ARGS__)

#ifdef _MSC_VER
#define TXT_ASSERT_FAIL __debugbreak()
#else
TXT_ASSERT_FAIL assert(false)
#endif

#define TXT_MOD_ASSERT(p, ...)                \
    do {                                      \
        if (!(p)) {                           \
            __VA_OPT__(TXT_LOG(__VA_ARGS__);) \
            TXT_ASSERT_FAIL;                  \
        }                                     \
    } while (false)

////////////////////////////////////////////////////////////////////////////////
// | TEXT MODS |
////////////////////////////////////////////////////////////////////////////////

namespace tm_parse {

using std::size_t;

constexpr size_t invalid_index_v = std::numeric_limits<size_t>::max();

// clang-format off

#ifdef TEXT_MODS_USE_WCHAR

using str      = std::wstring;
using str_view = std::wstring_view;
using txt_char = wchar_t;

#define TXT(str) L ## str

#else

using str      = std::string;
using str_view = std::string_view;
using txt_char = char;

#define TXT(str) str

#endif

using strstream = std::basic_stringstream<str::value_type>;

auto&& to_str(auto&& in_str) noexcept(false) {
    using T = std::decay_t<decltype(in_str)>;
    using CharType = T::value_type;

    if constexpr (std::is_same_v<CharType, str::value_type>) {
        return in_str;
    } else if constexpr (std::is_same_v<CharType, char> && std::is_same_v<str::value_type, wchar_t>) {
        return utils::widen(std::forward<T>(in_str));
    } else if constexpr (std::is_same_v<CharType, wchar_t> && std::is_same_v<str::value_type, char>) {
        return utils::narrow(std::forward<T>(in_str));
    } else {
        throw std::runtime_error{"unknown string type"};
    }
}

// clang-format on

////////////////////////////////////////////////////////////////////////////////
// | COMMON CHARACTERS |
////////////////////////////////////////////////////////////////////////////////

namespace txt {

// NOLINTBEGIN

////////////////////////////////////////////////////////////////////////////////
// | CHARACTERS |
////////////////////////////////////////////////////////////////////////////////

// #[[Character Literals]]
namespace lit {

// clang-format off

constexpr txt_char space = TXT(' ');
constexpr txt_char tab   = TXT('\t');
constexpr txt_char lf    = TXT('\n');
constexpr txt_char cr    = TXT('\r');

constexpr txt_char ws[]{ space, tab, lf, cr };

constexpr txt_char hash       = TXT('#');
constexpr txt_char underscore = TXT('_');
constexpr txt_char dquote     = TXT('\"');
constexpr txt_char squote     = TXT('\'');
constexpr txt_char hyphen     = TXT('-');

constexpr txt_char lparen   = TXT('(');
constexpr txt_char rparen   = TXT(')');
constexpr txt_char dot      = TXT('.');
constexpr txt_char colon    = TXT(':');
constexpr txt_char fslash   = TXT('/');
constexpr txt_char bslash   = TXT('\\');
constexpr txt_char star     = TXT('*');
constexpr txt_char comma    = TXT(',');
constexpr txt_char lbracket = TXT('[');
constexpr txt_char rbracket = TXT(']');
constexpr txt_char equal    = TXT('=');

constexpr txt_char zero  = TXT('0');
constexpr txt_char one   = TXT('1');
constexpr txt_char two   = TXT('2');
constexpr txt_char three = TXT('3');
constexpr txt_char four  = TXT('4');
constexpr txt_char five  = TXT('5');
constexpr txt_char six   = TXT('6');
constexpr txt_char seven = TXT('7');
constexpr txt_char eight = TXT('8');
constexpr txt_char nine  = TXT('9');

// This should never fail since unicode is backwards compatible with ascii
static_assert((lit::zero + 1) == lit::one);
static_assert((lit::one + 1) == lit::two);
static_assert((lit::two + 1) == lit::three);

// Same as above
static_assert((TXT('a') + 1) == TXT('b'));
static_assert((TXT('b') + 1) == TXT('c'));
static_assert((TXT('c') + 1) == TXT('d'));
static_assert((TXT('y') + 1) == TXT('z'));

static_assert((TXT('A') + 1) == TXT('B'));
static_assert((TXT('B') + 1) == TXT('C'));
static_assert((TXT('C') + 1) == TXT('D'));
static_assert((TXT('Y') + 1) == TXT('Z'));

// clang-format on

}  // namespace lit

////////////////////////////////////////////////////////////////////////////////
// | #[[Character Helper Functions]] |
////////////////////////////////////////////////////////////////////////////////

constexpr bool is_ws(txt_char ch) noexcept {
    return std::ranges::find(lit::ws, ch) != std::ranges::end(lit::ws);
}

constexpr bool isalpha(txt_char ch) noexcept {
    return (ch >= TXT('a') && ch <= TXT('z')) || (ch >= TXT('A') && ch <= TXT('Z'));
}

constexpr bool isdigit(txt_char ch) noexcept {
    return ch >= TXT('0') && ch <= TXT('9');
}

constexpr bool isalnum(txt_char ch) noexcept {
    return isalpha(ch) || isdigit(ch);
}

// These are fine, even though we don't actually care about anything except ascii
using std::tolower;
using std::toupper;

template <class... Chars>
constexpr bool any(txt_char ch, Chars... the_chars) noexcept {
    return ((ch == the_chars) || ...);
}

static_assert(any(lit::lf, lit::cr, lit::lf));
static_assert(!any(lit::space, lit::equal));

// NOLINTEND

}  // namespace txt

}  // namespace tm_parse