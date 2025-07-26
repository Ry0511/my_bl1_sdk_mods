//
// Date       : 21/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "text_mod/object_write_tree.h"
#include "text_mod/text_mod.h"

namespace tm_parse {

class TextModLoader {
   private:
    NameTable m_NameTable;
    std::deque<TextMod> m_TextMods;
    std::deque<FlatObjectWriteList> m_ObjWriteList;

   public:
    TextModLoader() = default;
    ~TextModLoader() = default;

   public:
    NameTable& name_table() { return m_NameTable; }

   public:
    // clang-format off
    decltype(auto) begin()       noexcept { return m_ObjWriteList.begin(); }
    decltype(auto) end()         noexcept { return m_ObjWriteList.end();   }
    decltype(auto) begin() const noexcept { return m_ObjWriteList.begin(); }
    decltype(auto) end()   const noexcept { return m_ObjWriteList.end();   }
    // clang-format on

   public:
    void load_from_file(const fs::path& pth);
    void load_from_str(str&& text);

   public:
    void unload_all();
    void reload_all();

   private:
    void _analyse_mod(const TextMod& mod);
    FlatObjectWriteList& _analyse_obj_def(const TextMod& mod, const rules::ObjectDefinitionRule& rule);
    FlatObjectWriteList& _find_or_create(table_ref obj_ref);
};

}  // namespace tm_parse
