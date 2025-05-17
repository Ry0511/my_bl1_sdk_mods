//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "text_mod_lexer.h"

namespace bl1_text_mods {

bool TextModLexer::read_number() noexcept {
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
        if (!dot_found && c == TXT('.') && next < m_Text.size() && std::isdigit(m_Text[next])) {
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

bool TextModLexer::read_identifier() noexcept {
    TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");
    if (!std::isalpha(m_Text[m_Position])) {
        return false;
    }

    read_while([](txt_char c) { return std::isalnum(c) || c == TXT('_'); });
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

bool TextModLexer::read_line_comment() noexcept {
    size_t start = m_Position;
    read_while([](txt_char c) { return c != TXT('\n') && c != TXT('\r'); });
    m_Token->Kind = TokenKind::LineComment;
    m_Token->Text = m_Text.substr(start, m_Position - start);
    return true;
}

bool TextModLexer::read_multiline_comment() noexcept(false) {
    TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");

    auto next = m_Position + 1;
    if (next < m_Text.size() && m_Text[next] == TXT('*')) {
        m_Position += 2;  // Skip past: /*

        auto pos = m_Text.find(TXT("*/"), m_Position);

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

bool TextModLexer::read_string_literal() noexcept(false) {
    m_Position++;
    // Does not handle escape sequences: \".*?\"
    read_while([](txt_char c) { return c != TXT('\"') && c != TXT('\n') && c != TXT('\r'); });

    if (this->eof()) {
        throw std::runtime_error("Unterminated string literal");
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
        throw std::runtime_error("Unterminated name literal");
    }
    m_Position++;  // Skip '
    m_Token->Kind = TokenKind::NameLiteral;
    m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
    return true;
}

bool TextModLexer::read_token(Token* token) {
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
            case TXT(' '):
            case TXT('\n'):
            case TXT('\r'):
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

                throw std::runtime_error(
                    std::format("Unknown character '{}' at position {}", str{peek()}, m_Position)
                );
            }
        }
    }

    return false;
}

}  // namespace bl1_text_mods