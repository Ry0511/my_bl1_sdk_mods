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
    if ((txt::isdigit(peek()) == 0) && peek() != lit::hyphen) {
        return false;
    }

    // If current char is a hyphen then the next char must be a digit
    if (peek() == lit::hyphen && (eof(1) || (txt::isdigit(peek(1)) == 0))) {
        throw_error_with_context("Expected digit after -");
    }

    // Consume
    const auto skip_digits = [this]() {
        while (txt::isdigit(peek())) {
            m_Position++;
        }
    };

    m_Position++;
    skip_digits();

    if (peek() == lit::dot) {
        if (eof(1) || (txt::isdigit(peek(1)) == 0)) {
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

    if (!(txt::isalpha(peek()) || peek() != lit::underscore)) {
        return false;
    }

    read_while([](txt_char c) { return txt::isalnum(c) || c == lit::underscore; });
    m_Token->Kind = TokenKind::Identifier;
    m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);

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

    // See if the token matches any known keywords if so adjust the token kind
    for (auto proxy : KeywordTokenIterator{}) {
        const str_view name = token_kind_names.at(proxy);
        if (icase_cmp(name, m_Token->Text)) {
            m_Token->Kind = proxy;
            return true;
        }
    }

    return true;
}

bool TextModLexer::read_line_comment() noexcept {
    read_while([](txt_char c) { return c != lit::lf && c != lit::cr; });
    m_Token->Kind = TokenKind::LineComment;
    m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
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
            m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
            return true;
        }
    }

    if (eof()) {
        throw_error_with_context("Unterminated multiline comment");
    }

    throw_error_with_context("Unknown lexing error");
}

bool TextModLexer::read_delegate_token() noexcept(false) {
    // - NOTE -
    // This is a bit of a hack but if we encounter a __ sequence we treat it the same as a single
    // line comment. The only reason for this work around is because its possible that an object
    // dump may include it and its trivial to handle.
    //
    // i.e., this is the result from UProperty::ExportText/ExportTextItem
    //   __OnSkillGradeChanged__Delegate=(null).None
    //

    if (eof(1) || peek(1) != lit::underscore) {
        throw_error_with_context("Delegate token must start with __");
    }

    // Consume the entire line
    read_while([](txt_char ch) -> bool { return ch != lit::lf; });

    const str_view line = m_Text.substr(m_Start, m_Position - m_Start);

    // Additional safety check
    if (line.find(TXT("__Delegate")) == str_view::npos) {
        throw_error_with_context("Delegate token must end with __Delegate");
    }

    m_Token->Kind = TokenKind::DelegateLine;
    m_Token->Text = line;

    return true;
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
    m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);

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
        throw_error_with_context("Encountered [\\r\\n] in string literal");
    }

    m_Position++;  // Skip "
    m_Token->Kind = TokenKind::StringLiteral;
    m_Token->Text = m_Text.substr(m_Start, m_Position - m_Start);
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
                return read_empty_lines();

            case TXT('\r'):
            case TXT('\t'):
            case TXT(' '):
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

            case TXT('_'):
                return read_delegate_token();

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