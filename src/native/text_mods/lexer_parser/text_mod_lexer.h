//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "text_mod_common.h"
#include "text_mod_tokens.h"

namespace bl1_text_mods {

class TextModLexer {
   private:
    str_view m_Text{};
    size_t m_Position{};
    size_t m_Start{};
    Token* m_Token{};

   public:
    explicit TextModLexer(const str_view& text) : m_Text(text), m_Position(0) {}
    ~TextModLexer() = default;

   public:
    /**
     * @return true if we are at the end of the stream.
     */
    [[nodiscard]] inline bool eof() const noexcept { return m_Position >= m_Text.size(); }

    /**
     * @return the current position in the stream.
     */
    [[nodiscard]] inline size_t position() const noexcept { return m_Position; }

    /**
     * Reads a single token from the stream.
     * @param token The token to write to, non-null
     * @return true if a token was read from the stream. If no token was read the returned token is
     *          set the EOF.
     */
    bool read_token(Token* token);

   private:
    /**
     * Advances the stream forward until the predicate returns false.
     * @param predicate The predicate to test against.
     */
    inline void read_while(auto&& predicate) {
        for (; m_Position < m_Text.size(); ++m_Position) {
            if (!predicate(m_Text[m_Position])) {
                return;
            }
        }
    }

    [[nodiscard]] inline str_view peek() const noexcept { return m_Text.substr(m_Position, 1); }

   private:
    inline bool read_simple(TokenKind kind) noexcept {
        m_Token->Kind = kind;
        m_Token->Text = m_Text.substr(m_Start, 1);
        m_Position++;
        return true;
    }
    bool read_number() noexcept;
    bool read_identifier() noexcept;
    bool read_line_comment() noexcept;
    bool read_multiline_comment() noexcept(false);
    bool read_string_literal() noexcept(false);
    bool read_name_literal() noexcept(false);
};

}  // namespace bl1_text_mods
