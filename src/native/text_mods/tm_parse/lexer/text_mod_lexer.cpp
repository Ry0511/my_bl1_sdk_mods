//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "text_mod_lexer.h"

namespace tm_parse {

namespace lit = txt::lit;

////////////////////////////////////////////////////////////////////////////////
// | LEXER |
////////////////////////////////////////////////////////////////////////////////

bool TextModLexer::read_number() noexcept(false) {
    TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");

    // Not a digit and not a minus
    if (!txt::isdigit(peek()) && peek() != lit::hyphen) {
        return false;
    }

    // If current char is a hyphen then the next char must be a digit
    if (peek() == lit::hyphen && (eof(1) || !txt::isdigit(peek(1)))) {
        throw_error_with_context("Expected digit after -");
    }

    // Consume
    const auto skip_digits = [this]() {
        while (!eof() && txt::isdigit(peek())) {
            m_Position++;
        }
    };

    m_Position++;
    skip_digits();

    if (!eof() && peek() == lit::dot) {
        if (eof(1) || !txt::isdigit(peek(1))) {
            throw_error_with_context("Expected digit after .");
        }
        m_Position++;
        skip_digits();
    }

    m_Token->Kind = TokenKind::Number;
    m_Token->TextRegion = {m_Start, m_Position - m_Start};
    return true;
}

bool TextModLexer::read_identifier() noexcept {
    TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");

    // Shoutouts to: bool _bUseAdvancedSettings;
    if (!txt::isalpha(peek()) && peek() != lit::underscore) {
        return false;
    }

    read_while([](txt_char c) { return txt::isalnum(c) || c == lit::underscore; });
    m_Token->Kind = TokenKind::Identifier;
    m_Token->TextRegion = {m_Start, m_Position - m_Start};

    // Case insensitive string comparison
    const auto icase_cmp = [](const str_view left, const str_view right) -> bool {
        if (left.size() != right.size()) {
            return false;
        }

        for (size_t i = 0; i < left.size(); ++i) {
            if (txt::tolower(left[i]) != txt::tolower(right[i])) {
                return false;
            }
        }
        return true;
    };

    // TODO: Minor optimisation if the length of the string is greater or less than the shortest and
    //        largest token we can shortcircuit.

    str_view text = m_Text.substr(m_Start, m_Position - m_Start);

    // Can't be any known token
    if (text.length() < min_token_len || text.length() > max_token_len) {
        return true;
    }

    // See if the token matches any known keywords if so adjust the token kind
    for (auto proxy : KeywordTokenIterator{}) {
        const str_view name = token_kind_names.at(proxy);
        if (icase_cmp(name, text)) {
            m_Token->Kind = proxy;
            return true;
        }
    }

    return true;
}

bool TextModLexer::read_line_comment() noexcept {
    read_while([](txt_char c) { return c != lit::lf && c != lit::cr; });
    m_Token->Kind = TokenKind::LineComment;
    m_Token->TextRegion = {m_Start, m_Position - m_Start};
    return true;
}

bool TextModLexer::read_multiline_comment() noexcept(false) {
    TXT_MOD_ASSERT(!this->eof(), "Unexpected end of input");

    if (eof(1) || peek(1) != lit::star) {
        throw_error_with_context("Expecting /*");
    }

    m_Position += 2;  // Skip /*

    for (; m_Position < m_Text.size(); ++m_Position) {
        const txt_char ch = peek();  // NOLINT(*-identifier-length)
        if (ch == lit::lf) {
            ++m_CurrentLine;
            continue;
        }

        if (ch == lit::star && !eof(1) && peek(1) == lit::fslash) {
            m_Position += 2;  // Skip */
            m_Token->Kind = TokenKind::MultiLineComment;
            m_Token->TextRegion = {m_Start, m_Position - m_Start};
            return true;
        }
    }

    if (eof()) {
        throw_error_with_context("Unterminated multiline comment");
    }

    throw_error_with_context("Unknown lexing error");
}

bool TextModLexer::read_empty_lines() noexcept(true) {
    TXT_MOD_ASSERT(!eof() && peek() == lit::lf, "Expected empty line");
    ++m_Position;
    ++m_CurrentLine;  // Count the initial line

    // NOLINTNEXTLINE(*-identifier-length)
    read_while([this](txt_char ch) -> bool {
        if (ch == lit::lf) {
            ++m_CurrentLine;
            return true;
        }

        return txt::any(ch, lit::space, lit::tab, lit::cr);
    });

    m_Token->Kind = TokenKind::BlankLine;
    m_Token->TextRegion = {m_Start, m_Position - m_Start};

    return true;
}

bool TextModLexer::read_string_literal() noexcept(false) {
    m_Position++;

    // Consume [^\"\r\n]+
    read_while([](txt_char c) { return !txt::any(c, lit::dquote, lit::lf, lit::cr); });

    // Consumed the entire stream
    if (this->eof()) {
        throw_error_with_context("Unterminated string literal", 0);
    }

    // Hit a [\r\n] character
    if (txt::any(peek(), lit::lf, lit::cr)) {
        throw_error_with_context("Encountered [\\r\\n] in string literal", 1);
    }

    m_Position++;  // Skip "
    m_Token->Kind = TokenKind::StringLiteral;
    m_Token->TextRegion = {m_Start, m_Position - m_Start};
    return true;
}

bool TextModLexer::read_name_literal() noexcept(false) {
    m_Position++;
    // Does not handle escape sequences: \".*?\"
    read_while([](txt_char c) { return !txt::any(c, lit::squote, lit::lf, lit::cr); });

    // Consumed the entire stream
    if (this->eof()) {
        throw_error_with_context("Unterminated name literal", 0);
    }

    // Hit a [\r\n] character
    if (txt::any(peek(), lit::lf, lit::cr)) {
        throw_error_with_context("Encountered [\\r\\n] in name literal");
    }

    m_Position++;  // Skip '
    m_Token->Kind = TokenKind::NameLiteral;
    m_Token->TextRegion = {m_Start, m_Position - m_Start};
    return true;
}

bool TextModLexer::read_token(Token* token) {
    // Always update this
    token->SourceStr = m_Text;

    if (this->eof()) {
        token->Kind = TokenKind::EndOfInput;
        token->TextRegion = {};
        return false;
    }

    TXT_MOD_ASSERT(token != nullptr, "Token should not be null");

    m_Token = token;
    m_Start = m_Position;

    for (; m_Position < m_Text.size(); ++m_Position) {
        switch (m_Text[m_Position]) {
            case lit::lf:
                return read_empty_lines();

            case lit::cr:
            case lit::tab:
            case lit::space:
                m_Start = m_Position + 1;
                continue;  // Skip

            case lit::lparen:
                return read_simple(TokenKind::LeftParen);

            case lit::rparen:
                return read_simple(TokenKind::RightParen);

            case lit::dot:
                return read_simple(TokenKind::Dot);

            case lit::comma:
                return read_simple(TokenKind::Comma);

            case lit::equal:
                return read_simple(TokenKind::Equal);

            case lit::colon:
                return read_simple(TokenKind::Colon);

            case lit::lbracket:
                return read_simple(TokenKind::LeftBracket);

            case lit::rbracket:
                return read_simple(TokenKind::RightBracket);

            case lit::hash:
                return read_line_comment();

            case lit::fslash:
                return read_multiline_comment();

            case lit::dquote:
                return read_string_literal();

            case lit::squote:
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