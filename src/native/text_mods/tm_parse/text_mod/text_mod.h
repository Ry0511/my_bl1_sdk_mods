//
// Date       : 21/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"
#include "parser/rules/primary_rules.h"

namespace tm_parse {

class TextMod {
   private:
    str m_Text;
    std::optional<fs::path> m_SourceFile;
    rules::ProgramRule m_Program;

   public:
    TextMod(const fs::path& pth) noexcept(false);
    TextMod(str mod_text) noexcept(false);

   public:
    ~TextMod() = default;

   public:
    TextMod(const TextMod&) = delete;
    TextMod& operator=(const TextMod&) = delete;
    TextMod& operator=(TextMod&&) = default;
    TextMod(TextMod&&) = default;

   public:
    void unload();
    void reload();

   public:
    str_view text() const noexcept { return m_Text; }
    const std::optional<fs::path>& source_file() const noexcept { return m_SourceFile; }
    const rules::ProgramRule& program() const noexcept { return m_Program; }

   private:
    void _parse_text();
    void _read_file();
};

}  // namespace tm_parse
