//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "text_mod_common.h"
#include "text_mod_tokens.h"

namespace bl1_text_mods {

class LexingError : std::runtime_error {
   private:
    size_t m_LineNumber{};
    size_t m_PosInLine{};
    str m_ErrorLine{};

   public:
    explicit LexingError(const std::string& msg) : std::runtime_error(msg) {}

    explicit LexingError(
        const std::string& msg,
        size_t line_number,
        size_t pos_in_line,
        str error_cause
    )
        : std::runtime_error(msg),
          m_LineNumber(line_number),
          m_PosInLine(pos_in_line),
          m_ErrorLine(std::move(error_cause)) {}

   public:
    [[nodiscard]] inline size_t line_number() const { return m_LineNumber; }
    [[nodiscard]] inline size_t pos_in_line() const { return m_PosInLine; }
    [[nodiscard]] inline str error_line() const { return m_ErrorLine; }
    [[nodiscard]] inline bool has_context() const { return !m_ErrorLine.empty(); }

   public:
    [[nodiscard]] std::array<str, 2> error_with_caret() const;

   public:
    using std::runtime_error::what;
};

class TextModLexer {
   private:
    str_view m_Text{};
    size_t m_Position{};
    size_t m_Start{};
    Token* m_Token{};

    size_t m_CurrentLine{0};

   public:
    explicit TextModLexer(const str_view& text) : m_Text(text), m_Position(0) {}
    ~TextModLexer() = default;

   public:
    /**
     * @return true if we are at the end of the stream.
     */
    [[nodiscard]] inline bool eof(int offset = 0) const noexcept {
        return (m_Position + offset) >= m_Text.size();
    }

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

    /**
     * Grabs the character from the current position in the stream.
     * @param offset offset into the stream.
     * @return The character at the position in the stream
     */
    [[nodiscard]] inline txt_char peek(int offset = 0) const noexcept {
        TXT_MOD_ASSERT((m_Position + offset) < m_Text.size(), "peek out of bounds");
        return m_Text[m_Position + offset];
    }

    /**
     * Gets the position of the first found line break.
     * @param search_forward Search direction.
     * @return The index of the first found line break. If no line break is found then the start or
     * end of the string is returned.
     */
    [[nodiscard]] inline size_t get_line_break_pos(bool search_forward) const noexcept {
        size_t pos = m_Position;
        while (pos > 0 && pos < m_Text.size() && m_Text[pos] != TXT('\n')) {
            search_forward ? pos++ : pos--;
        }
        return pos;
    }

    [[nodiscard]] inline void throw_error_with_context(const char* msg) const {
        size_t line_start = get_line_break_pos(false);
        size_t pos_in_line = m_Position - line_start;
        size_t end = get_line_break_pos(true);

        // Don't include the new line there
        if (end < m_Text.size() && m_Text[end] == '\n') {
            end--;
        }

        throw LexingError(
            std::format(
                "Lexing error {} at line {} at position {}",
                msg,
                m_CurrentLine,
                pos_in_line
            ),
            line_start,
            pos_in_line,
            str{m_Text.substr(line_start + 1, std::min(end - line_start, size_t{20}))}
        );
    }

   private:
    inline bool read_simple(TokenKind kind) noexcept {
        m_Token->Kind = kind;
        m_Token->Text = m_Text.substr(m_Start, 1);
        m_Position++;
        return true;
    }
    bool read_number() noexcept(false);
    bool read_identifier() noexcept;
    bool read_line_comment() noexcept;
    bool read_multiline_comment() noexcept(false);
    bool read_string_literal() noexcept(false);
    bool read_name_literal() noexcept(false);
};

}  // namespace bl1_text_mods
