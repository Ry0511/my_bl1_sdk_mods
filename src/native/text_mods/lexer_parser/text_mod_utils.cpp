//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "text_mod_utils.h"

namespace bl1_text_mods::utils {

std::string narrow(std::wstring_view wstr) {
    if (wstr.empty()) {
        return {};
    }

    auto num_chars = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        static_cast<int>(wstr.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );
    std::string ret(num_chars, '\0');
    if (WideCharToMultiByte(
            CP_UTF8,
            0,
            wstr.data(),
            static_cast<int>(wstr.size()),
            ret.data(),
            num_chars,
            nullptr,
            nullptr
        )
        != num_chars) {
        throw std::runtime_error("Failed to convert utf16 string!");
    }

    return ret;
}

std::wstring widen(std::string_view str) {
    if (str.empty()) {
        return {};
    }

    auto num_chars =
        MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring ret(num_chars, '\0');

    if (MultiByteToWideChar(
            CP_UTF8,
            0,
            str.data(),
            static_cast<int>(str.size()),
            ret.data(),
            num_chars
        )
        != num_chars) {
        throw std::runtime_error("Failed to convert utf8 string!");
    }

    return ret;
}

}  // namespace bl1_text_mods