//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "text_mod_lexer.h"

namespace tm_parse {

////////////////////////////////////////////////////////////////////////////////
// | LEXER |
////////////////////////////////////////////////////////////////////////////////

bool TextModLexer::read_number() noexcept(false) {
    TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");

    // Not a digit and not a minus
    if ((std::isdigit(peek()) == 0) && peek() != TXT('-')) {
        return false;
    }

    // If current char is a hyphen then the next char must be a digit
    if (peek() == TXT('-') && (eof(1) || (std::isdigit(peek(1)) == 0))) {
        throw_error_with_context("Expected digit after -");
    }

    // Consume
    const auto skip_digits = [this]() {
        while (std::isdigit(peek())) {
            m_Position++;
        }
    };

    m_Position++;
    skip_digits();

    if (peek() == TXT('.')) {
        if (eof(1) || (std::isdigit(peek(1)) == 0)) {
            throw_error_with_context("Expected digit after .");
        }
        m_Position++;
        skip_digits();
    }

    m_Token->Kind = TokenKind::Number;
    m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
    return true;
}

bool TextModLexer::read_identifier() noexcept {
    TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");

    if (std::isalpha(m_Text[m_Position]) == 0) {
        return false;
    }

    read_while([](txt_char c) { return std::isalnum(c) || c == TXT('_'); });
    m_Token->Kind = TokenKind::Identifier;
    m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);

    // Case insensitive string comparison
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

    // See if the token matches any known keywords if so adjust the token kind
    for (auto i = static_cast<int>(begin_kw_token); i <= static_cast<int>(end_kw_token); ++i) {
        const str_view& name = token_kind_names.at(i);
        if (icase_cmp(name, m_Token->Text)) {
            m_Token->Kind = static_cast<TokenKind>(i);
            return true;
        }
    }

    return true;
}

bool TextModLexer::read_line_comment() noexcept {
    const size_t start = m_Position;
    read_while([](txt_char c) { return c != TXT('\n') && c != TXT('\r'); });
    m_Token->Kind = TokenKind::LineComment;
    m_Token->Text = m_Text.substr(start, m_Position - start);
    return true;
}

bool TextModLexer::read_multiline_comment() noexcept(false) {
    TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");

    if (eof(1) || peek(1) != TXT('*')) {
        throw_error_with_context("Expecting /*");
    }

    m_Position += 2;  // Skip /*

    for (; m_Position < m_Text.size(); ++m_Position) {
        const txt_char ch = peek();  // NOLINT(*-identifier-length)
        if (ch == TXT('\n')) {
            ++m_CurrentLine;
            continue;
        }

        if (ch == TXT('*') && !eof(1) && peek(1) == TXT('/')) {
            m_Position += 2;  // Skip */
            m_Token->Kind = TokenKind::MultiLineComment;
            m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
            return true;
        }
    }

    if (eof()) {
        throw_error_with_context("Unterminated multiline comment");
    }

    throw_error_with_context("Unknown lexing error");
}

bool TextModLexer::read_string_literal() noexcept(false) {
    m_Position++;
    // Does not handle escape sequences: \".*?\"
    read_while([](txt_char c) { return c != TXT('\"') && c != TXT('\n') && c != TXT('\r'); });

    if (this->eof()) {
        throw_error_with_context("Unterminated string literal");
    }
    m_Position++;  // Skip "
    m_Token->Kind = TokenKind::StringLiteral;
    m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
    return true;
}

bool TextModLexer::read_name_literal() noexcept(false) {
    m_Position++;
    // Does not handle escape sequences: \".*?\"
    read_while([](txt_char c) { return c != TXT('\'') && c != TXT('\n') && c != TXT('\r'); });

    if (this->eof()) {
        throw_error_with_context("Unterminated name literal");
    }
    m_Position++;  // Skip '
    m_Token->Kind = TokenKind::NameLiteral;
    m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
    return true;
}

bool TextModLexer::read_token(Token* token) {
    if (this->eof()) {
        token->Kind = TokenKind::EndOfInput;
        token->Text = str_view{};
        return false;
    }

    TXT_MOD_ASSERT(token != nullptr, "Token should not be null");

    m_Token = token;
    m_Start = m_Position;

    for (; m_Position < m_Text.size(); ++m_Position) {
        switch (m_Text[m_Position]) {
            case TXT('\n'):
                ++m_CurrentLine;
            case TXT('\r'):
            case TXT(' '):
            case TXT('\t'):
                m_Start = m_Position + 1;
                continue;  // Skip

            case TXT('('):
                return read_simple(TokenKind::LeftParen);

            case TXT(')'):
                return read_simple(TokenKind::RightParen);

            case TXT('.'):
                return read_simple(TokenKind::Dot);

            case TXT(','):
                return read_simple(TokenKind::Comma);

            case TXT('='):
                return read_simple(TokenKind::Equal);

            case TXT(':'):
                return read_simple(TokenKind::Colon);

            case TXT('['):
                return read_simple(TokenKind::LeftBracket);

            case TXT(']'):
                return read_simple(TokenKind::RightBracket);

            case TXT('#'):
                return read_line_comment();

            case TXT('/'):
                return read_multiline_comment();

            case TXT('\"'):
                return read_string_literal();

            case TXT('\''):
                return read_name_literal();

            default: {
                if (read_number()) {
                    return true;
                }

                if (read_identifier()) {
                    return true;
                }

                // Anything unknown should be an assertion error but I can't guarantee this
                //  currently since there is a knowledge gap.
                throw_error_with_context("Unknown token");
            }
        }
    }

    // Pretty sure this can't be reached
    return false;
}

}  // namespace tm_parse