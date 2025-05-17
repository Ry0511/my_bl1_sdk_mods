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
    std::size_t m_Position{};
    std::size_t m_Start{};
    Token* m_Token{};

   public:
    explicit TextModLexer(const str_view& text) : m_Text(text), m_Position(0) {}
    ~TextModLexer() = default;

   private:
    bool read_simple(TokenKind kind) {
        m_Token->Kind = kind;
        m_Token->Text = m_Text.substr(m_Start, 1);
        m_Position++;
        return true;
    }

    bool read_number() noexcept {
        TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");

        if (!std::isdigit(m_Text[m_Position])) {
            return false;
        }

        // Parse as: [0-9]+ (. [0-9]+)?
        bool dot_found = false;

        // Advance stream
        read_while([this, &dot_found](txt_char c) {
            size_t next = m_Position + 1;

            // Dot not found, current char is a dot, and next char is a digit
            if (!dot_found && c == TEXT('.') && next < m_Text.size()
                && std::isdigit(m_Text[next])) {
                dot_found = true;
                return true;
            }

            // Continue while a digit
            return std::isdigit(c) != 0;
        });

        m_Token->Kind = TokenKind::Number;
        m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
        return true;
    }

    bool read_identifier() noexcept {
        TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");
        if (!std::isalpha(m_Text[m_Position])) {
            return false;
        }

        read_while([](txt_char c) { return std::isalnum(c) || c == TEXT('_'); });
        m_Token->Kind = TokenKind::Identifier;
        m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);

        const auto icase_cmp = [](str_view left, str_view right) -> bool {
            if (left.size() != right.size()) {
                return false;
            }

            for (size_t i = 0; i < left.size(); ++i) {
                if (std::tolower(left[i]) != std::tolower(right[i])) {
                    return false;
                }
            }
            return true;
        };

        // See if the token matches any known keywords
        for (auto i = begin_kw_token; i <= end_kw_token; ++i) {
            const str_view& name = token_kind_names[i];
            if (icase_cmp(name, m_Token->Text)) {
                m_Token->Kind = static_cast<TokenKind>(i);
                return true;
            }
        }

        return true;
    }

    bool read_line_comment() noexcept {
        size_t start = m_Position;
        read_while([](txt_char c) { return c != TEXT('\n') && c != TEXT('\r'); });
        m_Token->Kind = TokenKind::LineComment;
        m_Token->Text = m_Text.substr(start, m_Position - start);
        return true;
    }

    bool read_multiline_comment() noexcept(false) {
        TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");

        auto next = m_Position + 1;
        if (next < m_Text.size() && m_Text[next] == TEXT('*')) {
            m_Position += 2;  // Skip past: /*

            auto pos = m_Text.find(TEXT("*/"), m_Position);

            // Have we found the: */
            if (pos != str::npos) {
                m_Token->Kind = TokenKind::MultiLineComment;
                m_Token->Text = m_Text.substr(m_Start, (pos - m_Start) + 2);
                m_Position = pos + 2;
                return true;
            }

            throw std::runtime_error("Unterminated multi-line comment");
        }
        throw std::runtime_error("Invalid multiline comment sequence / expecting /*");
    }

    bool read_string_literal() noexcept(false) {
        m_Position++;
        // Does not handle escape sequences: \".*?\"
        read_while([](txt_char c) { return c != TEXT('\"') && c != TEXT('\n') && c != TEXT('\r'); }
        );

        if (this->eof()) {
            throw std::runtime_error("Unterminated string literal");
        }
        m_Position++;  // Skip "
        m_Token->Kind = TokenKind::StringLiteral;
        m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
        return true;
    }

    bool read_name_literal() noexcept(false) {
        m_Position++;
        // Does not handle escape sequences: \".*?\"
        read_while([](txt_char c) { return c != TEXT('\'') && c != TEXT('\n') && c != TEXT('\r'); }
        );

        if (this->eof()) {
            throw std::runtime_error("Unterminated name literal");
        }
        m_Position++;  // Skip "
        m_Token->Kind = TokenKind::NameLiteral;
        m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
        return true;
    }

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

   public:
    /**
     * @return true if we are at the end of the stream.
     */
    [[nodiscard]] bool eof() const noexcept { return m_Position >= m_Text.size(); }

    /**
     * Reads a single token from the stream.
     * @param token The token to write to, non-null
     * @return true if a token was read from the stream. If no token was read the returned token is
     *          set the EOF.
     */
    bool read_token(Token* token) {
        if (this->eof()) {
            // eof
            token->Kind = TokenKind::END_OF_INPUT;
            token->Text = str_view{};
            return false;
        }

        TXT_MOD_ASSERT(token != nullptr, "Token should not be null");

        m_Token = token;
        m_Start = m_Position;

        for (; m_Position < m_Text.size(); ++m_Position) {
            switch (m_Text[m_Position]) {
                case TEXT(' '):
                case TEXT('\n'):
                case TEXT('\r'):
                case TEXT('\t'):
                    m_Start = m_Position + 1;
                    continue;  // Skip

                case TEXT('('):
                    return read_simple(TokenKind::LeftParen);

                case TEXT(')'):
                    return read_simple(TokenKind::RightParen);

                case TEXT('.'):
                    return read_simple(TokenKind::Dot);

                case TEXT(','):
                    return read_simple(TokenKind::Comma);

                case TEXT('='):
                    return read_simple(TokenKind::Equal);

                case TEXT('['):
                    return read_simple(TokenKind::LeftBracket);

                case TEXT(']'):
                    return read_simple(TokenKind::RightBracket);

                case TEXT('#'):
                    return read_line_comment();

                case TEXT('/'):
                    return read_multiline_comment();

                case TEXT('\"'):
                    return read_string_literal();

                case TEXT('\''):
                    return read_name_literal();

                default: {
                    if (read_number()) {
                        return true;
                    }

                    if (read_identifier()) {
                        return true;
                    }
                }
            }
        }

        return false;
    }
};

}
