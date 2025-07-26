//
// Date       : 21/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "parser/rules/primary_rules.h"
#include "text_mod/text_mod_loader.h"

namespace tm_parse {

using namespace rules;

void TextModLoader::load_from_file(const fs::path& pth) {
    if (!fs::is_regular_file(pth)) {
        throw std::runtime_error{std::format("file not found '{}'", pth.string())};
    }
    _analyse_mod(m_TextMods.emplace_back(pth));
}

void TextModLoader::load_from_str(str&& text) {
    _analyse_mod(m_TextMods.emplace_back(std::move(text)));
}

void TextModLoader::unload_all() {
    m_ObjWriteList = {};
    for (TextMod& mod : m_TextMods) {
        mod.unload();
    }
}

void TextModLoader::reload_all() {
    m_ObjWriteList = {};
    for (TextMod& mod : m_TextMods) {
        mod.reload();
        _analyse_mod(mod);
    }
}

FlatObjectWriteList& TextModLoader::_find_or_create(table_ref obj_ref) {
    for (FlatObjectWriteList& ls : m_ObjWriteList) {
        if (ls.obj_ref() == obj_ref) {
            return ls;
        }
    }
    return m_ObjWriteList.emplace_back(obj_ref);
}

void TextModLoader::_analyse_mod(const TextMod& mod) {
    NameTable& table = m_NameTable;

    for (const auto& inner : mod.program().rules()) {
        if (std::holds_alternative<SetCommandRule>(inner)) {
            const auto& rule = std::get<SetCommandRule>(inner);
            table_ref obj_ref = table.register_name(rule.object().to_str(mod.text()));
            FlatObjectWriteList& ls = _find_or_create(obj_ref);
            ls.add_set_command(*this, mod, rule);
        } else if (std::holds_alternative<ObjectDefinitionRule>(inner)) {
            _analyse_obj_def(mod, std::get<ObjectDefinitionRule>(inner));
        }
    }
}

FlatObjectWriteList& TextModLoader::_analyse_obj_def(const TextMod& mod, const rules::ObjectDefinitionRule& rule) {
    NameTable& table = m_NameTable;

    table_ref obj_ref = table.register_name(rule.name().to_str(mod.text()));

    // Obtain the referenced object or create it
    FlatObjectWriteList& ls = _find_or_create(obj_ref);

    // Process sub objects
    for (const auto& inner : rule.child_objects()) {
        ls.add_child_obj(_analyse_obj_def(mod, *inner));
    }

    // Process the object
    ls.add_obj_def(*this, mod, rule);
    return ls;
}

}  // namespace tm_parse