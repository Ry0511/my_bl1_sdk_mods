//
// Date       : 29/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/parser_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

str_view DotIdentifierRule::to_string(const TextModParser& parser) const noexcept {
    if (!this->operator bool()) {
        return str_view{};
    }

    // Single identifier token
    if (m_StartIndex == m_EndIndex) {
        str_view full_text = parser.m_Lexer->text();
        return parser.m_Regions[m_StartIndex].view_from(full_text);
    }

    return parser.get_substr_from_index(m_StartIndex, m_EndIndex);
}

str_view ObjectIdentifierRule::to_string(const TextModParser& parser) const noexcept {
    if (!m_PrimaryIdentifier) {
        return str_view{};
    }

    if (!m_SecondaryIdentifier) {
        return m_PrimaryIdentifier.to_string(parser);
    }

    return parser.get_substr_from_index(m_PrimaryIdentifier.start_index(), m_SecondaryIdentifier.end_index());
}

str_view ArrayAccessRule::to_string(const TextModParser& parser) const noexcept {
    if (!this->operator bool()) {
        return str_view{};
    }

    return parser.get_substr_from_index(this->open_token_index(), this->close_token_index());
}

str_view PropertyAccessRule::to_string(const TextModParser& parser) const noexcept {
    if (!this->operator bool()) {
        return str_view{};
    }

    str_view txt = parser.m_Lexer->text();
    const auto& region = parser.get_region(this->IdentifierTokenIndex);
    return txt.substr(this->IdentifierTokenIndex, txt.size() - this->IdentifierTokenIndex);
}

str_view SetCommandRule::to_string(const TextModParser& parser) const noexcept {

    const auto& prop = m_PropertyAccess;
    size_t start = m_SetCommandIndex;

    if (prop.array_access()) {
        return parser.get_substr_from_index(start, prop.array_access().close_token_index());
    }

    return parser.get_substr_from_index(m_SetCommandIndex, m_PropertyAccess.identifier_token_index());
}

}  // namespace tm_parse::rules