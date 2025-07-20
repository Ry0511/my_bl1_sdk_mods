//
// Date       : 20/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "text_mod/name_table.h"
#include "parser/rules/primary_rules.h"

namespace tm_parse {

class TextModContext {
   private:
    NameTable m_NameTable;
    rules::ProgramRule m_Program;
    std::unique_ptr<str> m_LocalStr;

   public:
    TextModContext() = default;
    ~TextModContext() = default;

   public:
    operator bool() const noexcept { return m_Program.operator bool(); }

   public:
    const rules::ProgramRule& program() const { return m_Program; }
    rules::ProgramRule& program() { return m_Program; }
    const NameTable& name_table() const { return m_NameTable; }
    NameTable& name_table() { return m_NameTable; }

   public:
    void load_from_file(const fs::path& pth);
    void load_from_str(str_view vw);
};

}  // namespace tm_parse