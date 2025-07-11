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

   private:
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

    size_t get_line_start(size_t pos) const noexcept {
        if (size() < pos) {
            return invalid_index_v;
        }

        if (pos == 0) {
            return 0;
        }

        size_t line_start = pos;
        do {
            if (m_Text[line_start] == TXT('\n')) {
                return line_start + 1;
            }
            --line_start;
        } while (line_start > 0);

        if (m_Text[line_start] == TXT('\n')) {
            return line_start + 1;
        }

        return 0;
    }

    size_t get_line_end(size_t pos) const noexcept {
        if (size() < pos) {
            return invalid_index_v;
        }

        if (pos == text().size() - 1) {
            return pos;
        }

        size_t line_end = pos;
        do {
            if (m_Text[line_end] == TXT('\n')) {
                return line_end;
            }
            ++line_end;
        } while (line_end < text().size());

        return text().size() - 1;
    }

    size_t get_line_start(const TokenTextView& vw) const noexcept { return get_line_start(vw.Start); }

    TokenTextView get_line(size_t pos) const noexcept {
        size_t line_start = get_line_start(pos);
        size_t line_end = get_line_end(pos);
        return TokenTextView{line_start, line_end - line_start + 1};
    }

    [[nodiscard]] size_t get_line_number(size_t pos) const noexcept {
        size_t line_count = 0;

        for (size_t i = 0; i < pos; i++) {
            if (m_Text[i] == TXT('\n')) {
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
    [[noreturn]] void throw_error_with_context(const char* msg, int context_size = 3) {
        const size_t line_start = get_line_break_pos(false);

        // Removes starting and ending new lines
        const auto trim_line = [](str_view line) -> str_view {
            if (line.empty()) {
                return line;
            }

            if (line.front() == TXT('\n')) {
                line = line.substr(1);
            }

            if (!line.empty() && line.back() == TXT('\n')) {
                line = line.substr(0, line.size() - 1);
            }

            return line;
        };

        // NOTE: ErrorWithContext assumes the end of the line is the start of the error. It's only
        //        for providing basic error info to help debug the lexer.
        const str_view error_snippet = trim_line(m_Text.substr(line_start, (m_Position - line_start) + 1));

        // Didn't ask for more context
        if (context_size < 1) {
            size_t pos = m_Position;
            throw ErrorWithContext(msg, get_line_number(pos), pos, str{error_snippet});
        }

        using LineContext = ErrorWithContext::LineContext;
        LineContext context = std::make_optional<LineContext::value_type>();

        {
            int last = static_cast<int>(line_start);
            int pos = (line_start != 0) ? static_cast<int>(line_start) - 1 : 0;  // Exclude \n

            for (int i = 0; i < context_size; i++) {
                TXT_MOD_ASSERT(pos >= 0, "{}", pos);

                // Walk backwards to the first \n character
                while (pos > 0 && m_Text[pos] != TXT('\n')) {
                    pos--;
                }

                // Grab line excluding starting/ending new line chars
                const str_view line = trim_line(m_Text.substr(pos, (last - pos)));

                context->emplace_front(line);
                last = pos;

                // Since: m_Text[pos] == '\n' move back
                if (pos > 0) {
                    pos--;
                } else {
                    break;
                }
            }
        }

        size_t pos = m_Position;
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
