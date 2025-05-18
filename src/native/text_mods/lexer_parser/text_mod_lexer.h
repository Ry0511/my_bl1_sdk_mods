//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "text_mod_common.h"
#include "text_mod_errors.h"
#include "text_mod_tokens.h"

namespace bl1_text_mods {

class TextModLexer {
    // TODO: Replace m_Text str_view with a text stream for lazy lexing
   private:
    str_view m_Text;
    size_t m_Position{0};
    size_t m_Start{0};
    Token* m_Token{nullptr};

    size_t m_CurrentLine{0};

   public:
    explicit TextModLexer(const str_view& text) : m_Text(text) {}
    ~TextModLexer() = default;

   public:
    void reset() {
        m_Position = 0;
        m_Start = 0;
        m_CurrentLine = 0;
        m_Token = nullptr;
    }

    /**
     * @return Current text stream
     */
    [[nodiscard]] const str_view& text() const noexcept { return m_Text; }

    /**
     * @return Current text stream size; equivalent to text().size()
     */
    [[nodiscard]] size_t size() const noexcept { return m_Text.size(); }

   public:
    /**
     * @param offset Offset to current stream position.
     * @return True if the current position + offset is beyond the stream
     */
    [[nodiscard]] bool eof(int offset = 0) const noexcept { return (m_Position + offset) >= m_Text.size(); }

    /**
     * @return Current position in the stream
     */
    [[nodiscard]] size_t position() const noexcept { return m_Position; }

    /**
     * @return Current line number; Zero based.
     */
    [[nodiscard]] size_t line() const noexcept { return m_CurrentLine; }

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
    void read_while(auto&& predicate) {
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
    [[nodiscard]] txt_char peek(int offset = 0) const noexcept {
        TXT_MOD_ASSERT((m_Position + offset) < m_Text.size(), "peek out of bounds");
        return m_Text[m_Position + offset];
    }

    /**
     * Gets the position of the first found line break.
     * @param search_forward Search direction.
     * @return The index of the first found line break. If no line break is found then the start or
     * end of the string is returned.
     */
    [[nodiscard]] size_t get_line_break_pos(bool search_forward) const noexcept {
        // TODO: this is effectively get_line_start, get_line_end
        size_t pos = m_Position;
        while (pos > 0 && pos < m_Text.size() && m_Text[pos] != TXT('\n')) {
            search_forward ? pos++ : pos--;
        }
        return pos;
    }

    /**
     * Uses the current start and position to gather context information and throws an
     * ErrorWithContext exception.
     * @param msg The main error message.
     * @param context_size The size of the context, that is, how many lines before the error we
     *                      capture.
     */
    [[noreturn]] void throw_error_with_context(const char* msg, int context_size = 3) const {
        size_t line_start = get_line_break_pos(false);

        if (line_start != 0) {
            line_start++;
        }

        // From start of the line to the error; We don't include the content after but we probably should
        const str_view error_snippet = m_Text.substr(line_start, (m_Position - line_start) + 1);

        // Didn't ask for more context
        if (context_size < 1) {
            throw ErrorWithContext(msg, m_CurrentLine, m_Position, str{error_snippet});
        }

        using LineContext = ErrorWithContext::LineContext;
        LineContext context = std::make_optional<LineContext::value_type>();
        // TODO: Get last N lines from stream

        throw ErrorWithContext(msg, m_CurrentLine, m_Position, str{error_snippet}, context);
    }

   private:
    bool read_simple(TokenKind kind) noexcept {
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
