//
// Date       : 21/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"
#include "text_mod/text_mod_loader.h"

namespace tm_parse {

void TextModLoader::load_from_file(const fs::path& pth) {
    if (!fs::is_regular_file(pth)) {
        throw std::runtime_error{std::format("file not found '{}'", pth.string())};
    }
    m_TextMods.emplace_back(pth);
}

void TextModLoader::load_from_str(const str& text) {
    m_TextMods.emplace_back(text);
}

void TextModLoader::unload_all() {
    for (TextMod& mod : m_TextMods) {
        mod.unload();
    }
}

void TextModLoader::reload_all() {
    for (TextMod& mod : m_TextMods) {
        mod.reload();
    }
}

}  // namespace tm_parse