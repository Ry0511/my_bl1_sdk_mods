//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#if defined(TEXT_MODS_UNREALSDK)
#include "unrealsdk/utils.h"
#endif

/**

Note that this was mostly taken from: (cpp file is basically copy pasted)
  .../unrealsdk/src/unrealsdk/utils.cpp

 */

namespace tm_parse::utils {

static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t is different size to char16_t");

#if defined(TEXT_MODS_STANDALONE)

std::string narrow(std::wstring_view wstr) noexcept(false);
std::wstring widen(std::string_view str) noexcept(false);

#else

using namespace unrealsdk::utils;

#endif

decltype(auto) to_str(auto&& vw) noexcept(false) {
    using T = std::decay_t<decltype(vw)>;

    if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, std::wstring_view>) {
        return narrow(std::wstring_view{vw});
    } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
        return widen(std::string_view{vw});
    } else {
        throw std::runtime_error("unsupported");
    }
}

}  // namespace tm_parse::utils

#if defined(TEXT_MODS_STANDALONE)

template <>
struct std::formatter<std::wstring> : std::formatter<std::string> {
    auto format(const std::wstring& str, std::format_context& ctx) const {
        using namespace tm_parse;
        return formatter<std::string>::format(utils::narrow(str), ctx);
    }
};

#endif