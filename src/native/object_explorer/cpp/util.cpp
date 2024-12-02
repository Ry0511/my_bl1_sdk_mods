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

std::wstring str_to_wstr(const std::string& str) noexcept {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

}  // namespace object_explorer
