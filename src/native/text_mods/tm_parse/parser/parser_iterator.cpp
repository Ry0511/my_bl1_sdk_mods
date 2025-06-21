//
// Date       : 21/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/parser_iterator.h"
#include "parser/text_mod_parser.h"

namespace tm_parse {

ParserIterator& ParserIterator::operator++() {
    if (m_Index >= m_Parser->m_Tokens.size() && m_Parser->m_EndOfInputReached) {
        return *this;
    }

    m_Index++;

    if (!parser()->fetch_to_fit(m_Index)) {
        return *this;
    }

    if (is_skip_blank_lines()) {
        while (operator*() == TokenKind::BlankLine) {
            m_Index++;
            m_Parser->fetch_to_fit(m_Index);
        }
    }

    return *this;
}

ParserIterator& ParserIterator::operator--() {
    if (m_Index > 0) {
        m_Index--;
    }

    if (is_skip_blank_lines()) {
        while (operator*() == TokenKind::BlankLine) {
            m_Index--;
            m_Parser->fetch_to_fit(m_Index);
        }
    }

    return *this;
}

const Token& ParserIterator::operator*() const noexcept {
    parser()->fetch_to_fit(m_Index);
    return m_Parser->get_token(m_Index);
}

bool ParserIterator::operator==(TokenKind kind) const noexcept {
    const Token& token = operator*();

    if (kind == TokenKind::Identifier && is_coalesce()) {
        return token.is_identifier();
    }

    return token == kind;
}

bool ParserIterator::operator!=(TokenKind kind) const noexcept {
    return !operator==(kind);
}

}  // namespace tm_parse
