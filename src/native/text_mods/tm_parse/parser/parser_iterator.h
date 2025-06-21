//
// Date       : 21/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "common/text_mod_common.h"

namespace tm_parse {

class Token;
class TextModParser;

class ParserIterator {
   private:
    friend TextModParser;

   private:
    TextModParser* m_Parser;
    size_t m_Index;
    bool m_SkipBlankLines;
    bool m_Coalesce;

   public:
    ParserIterator(TextModParser* parser, size_t index, bool skip_blank_lines, bool coalesce_identifiers) noexcept
        : m_Parser(parser),
          m_Index(index),
          m_SkipBlankLines(skip_blank_lines),
          m_Coalesce(coalesce_identifiers) {};

    ~ParserIterator() = default;

   public:  // clang-format off
    TextModParser* parser()    const noexcept { return m_Parser;         }
    size_t index()             const noexcept { return m_Index;          }
    bool is_skip_blank_lines() const noexcept { return m_SkipBlankLines; }
    bool is_coalesce()         const noexcept { return m_Coalesce;       }
    // clang-format on

    void set_skip_blank_lines(bool skip_blank_lines) noexcept { m_SkipBlankLines = skip_blank_lines; }
    void set_coalesce(bool coalesce) noexcept { m_Coalesce = coalesce; }

   public:
    const Token* operator->() const noexcept { return &operator*(); }

   public:
    ParserIterator& operator++();
    ParserIterator& operator--();
    const Token& operator*() const noexcept;

   public:
    bool operator==(TokenKind kind) const noexcept;
    bool operator!=(TokenKind kind) const noexcept;
};

}  // namespace tm_parse
