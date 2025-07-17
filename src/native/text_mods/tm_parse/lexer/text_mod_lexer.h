//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "common/text_mod_common.h"
#include "common/text_mod_errors.h"
#include "common/text_mod_tokens.h"

namespace tm_parse {

// #[[TextModLexer]]
class TextModLexer {
   private:
    struct LexerState {
        size_t Start, Position;
    };

   private:
    str_view m_Text;
    size_t m_Position{0};
    size_t m_Start{0};
    Token* m_Token{nullptr};
    LexerState m_State{};

   public:
    explicit TextModLexer(str_view text) : m_Text(text) {}
    ~TextModLexer() = default;

   public:
    void reset() {
        m_Position = 0;
        m_Start = 0;
        m_Token = nullptr;
    }

    /**
     * @return Current text stream
     */
    [[nodiscard]] str_view text() const noexcept { return m_Text; }

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

    size_t get_line_start(size_t pos) const {
        const str_view tx = text();
        if (tx.empty()) {
            return 0;
        }
        using namespace txt;

        pos = std::min(pos, size() - 1);

        while (pos > 0 && pos < tx.size() && tx[pos] != lit::lf) {
            --pos;
        }

        // Reached start of stream before any new lines
        if (pos == 0 && tx[pos] != lit::lf) {
            return pos;
        }

        // TODO: Verify what happens if the first character in the stream is a new line
        return pos + 1;
    }

    size_t get_line_end(size_t pos) const {
        const str_view tx = text();
        using namespace txt;
        if (tx.empty()) {
            return 0;
        }
        pos = std::min(pos, size() - 1);

        while (pos > 0 && pos < tx.size() && tx[pos] != txt::lit::lf) {
            ++pos;
        }

        return pos;
    }

    size_t get_line_start(const TokenTextView& vw) const { return get_line_start(vw.Start); }

    TokenTextView get_line(size_t pos) const {
        size_t line_start = get_line_start(pos);
        size_t line_end = get_line_end(pos);

        return TokenTextView{line_start, line_end - line_start};
    }

    [[nodiscard]] size_t get_line_number(size_t pos) const noexcept {
        size_t line_count = 1;  // 1 based

        const str_view tx = text();
        pos = std::min(pos, size());

        for (size_t i = 0; i < pos; i++) {
            if (tx[i] == txt::lit::lf) {
                line_count++;
            }
        }
        return line_count;
    }

    [[nodiscard]] size_t get_line_number(const TokenTextView& vw) const noexcept { return get_line_number(vw.Start); }

    /**
     * Skips the provided number of characters.
     * @param count The number of characters to skip
     */
    void skip(size_t count) { m_Position = std::min(m_Position + count, size()); }

    /**
     * Reads a single token from the stream.
     * @param token The token to write to, non-null
     * @return true if a token was read from the stream. If no token was read the returned token is
     *          set the EOF.
     */
    bool read_token(Token* token);

    void save() noexcept { m_State = LexerState{.Start = m_Start, .Position = m_Position}; }

    void restore() noexcept {
        m_Position = m_State.Position;
        m_Start = m_State.Start;
    }

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
     * Uses the current start and position to gather context information and throws an
     * ErrorWithContext exception.
     * @param msg The main error message.
     * @param context_size The size of the context, that is, how many lines before the error we
     *                      capture.
     */
    [[noreturn]] void throw_error_with_context(const char* msg, int context_size = 3) {
        size_t pos = std::min(m_Position, size() - 1);

        // NOTE: ErrorWithContext assumes the end of the line is the start of the error. It's only
        //        for providing basic error info to help debug the lexer.
        const str_view error_snippet = get_line(pos).view_from(text());

        // Didn't ask for more context
        if (context_size < 1) {
            throw ErrorWithContext(msg, get_line_number(pos), pos, str{error_snippet});
        }

        using LineContext = ErrorWithContext::LineContext;
        LineContext context = std::make_optional<LineContext::value_type>();

        {
            size_t line_start = get_line_start(m_Position);

            // Ignore current line as it is the error line
            if (line_start > 1) {
                line_start = get_line_start(line_start - 2);
            }

            int remaining = context_size;
            do {
                // Grab line & push front
                context.value().push_front(str{get_line(line_start).view_from(text())});

                // -1 is the \n and get_line_start will terminate on \n so we need to get before that
                if (line_start > 1) {
                    line_start = get_line_start(line_start - 2);
                }

                --remaining;
            } while (line_start > 0 && remaining > 0);
        }

        throw ErrorWithContext(msg, get_line_number(pos), pos, str{error_snippet}, context);
    }

   private:
    bool read_simple(TokenKind kind) noexcept {
        m_Token->Kind = kind;
        m_Token->TextRegion = {m_Start, 1};
        m_Position++;
        return true;
    }
    bool read_number() noexcept(false);
    bool read_identifier() noexcept;
    bool read_line_comment() noexcept;
    bool read_multiline_comment() noexcept(false);
    bool read_empty_lines() noexcept(true);
    bool read_string_literal() noexcept(false);
    bool read_name_literal() noexcept(false);
};

}  // namespace tm_parse
