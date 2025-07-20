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

// TODO
//   Need to revisit this as it has caused a few issues which really could've been avoided if we
//   stuck to a single string type. The original intention is still something I want to support but
//   I don't know how useful it will actually be.
//

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

using str_istreambuf_it = std::istreambuf_iterator<str::value_type>;
using str_ostreambuf_it = std::ostreambuf_iterator<str::value_type>;

using str_ifstream = std::basic_ifstream<str::value_type>;
using str_fstream = std::basic_fstream<str::value_type>;
using str_istream = std::basic_istream<str::value_type>;

template<class R>
decltype(auto) to_str(auto&& in) {
    using InType = typename std::remove_cvref_t<std::decay_t<decltype(in)>>;
    using CharType = typename InType::value_type;
    static_assert(std::disjunction_v<std::is_same<CharType, char>, std::is_same<CharType, wchar_t>>, "expecting wchar_t or char string");

    constexpr bool is_string = std::is_same_v<InType, std::string> || std::is_same_v<InType, std::string_view>;
    constexpr bool is_wstring = std::is_same_v<InType, std::wstring> || std::is_same_v<InType, std::wstring_view>;

    // str -> str
    if constexpr (std::is_same_v<InType, R>) {
        return std::forward<decltype(in)>(in);
    }
    // string -> wstring
    else if constexpr (is_string && std::is_same_v<R, std::wstring>) {
        return utils::widen(std::string_view{in});
    }
    // wstring -> string
    else if constexpr (is_wstring && std::is_same_v<R, std::string>) {
        return utils::narrow(std::wstring_view{in});
    }
    // Shouldn't ever happen
    else {
        throw std::runtime_error{"unknown conversion"};
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