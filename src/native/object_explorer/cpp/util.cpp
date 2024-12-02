//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "object_explorer.h"

#include "windows.h"

namespace object_explorer {

std::string wstr_to_str(const std::wstring& wstr) noexcept {
    int buffer_size = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        static_cast<int>(wstr.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (buffer_size == 0) {
        return std::string{};
    }

    std::string utf8_str(buffer_size, '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        static_cast<int>(wstr.size()),
        utf8_str.data(),
        buffer_size,
        nullptr,
        nullptr
    );

    return utf8_str;
}

}  // namespace object_explorer
