//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "common/text_mod_common.h"
#include "lexer/text_mod_lexer.h"
#include "parser/parser_rules.h"

namespace tm_parse {

using namespace rules;

class TextModParser {
   private:
    friend ParserBaseRule;

   private:
    TextModLexer* m_Lexer;
    std::vector<Token> m_Tokens;
    size_t m_Index;
    bool m_EndOfInputReached;

   public:
    explicit TextModParser(TextModLexer* lexer) noexcept;
    ~TextModParser() = default;

    ////////////////////////////////////////////////////////////////////////////////
    // | UTILITY |
    ////////////////////////////////////////////////////////////////////////////////

   public:
    const Token& get_token(size_t index) noexcept(true) {
        if (index >= m_Tokens.size()) {
            return token_eof;
        }
        return m_Tokens[index];
    }

    size_t index() const noexcept { return m_Index; }
    bool eof() const noexcept { return m_EndOfInputReached; }

    ////////////////////////////////////////////////////////////////////////////////
    // | PRIMARY OPERATIONS |
    ////////////////////////////////////////////////////////////////////////////////

   public:
    SetCommandRule parse_set_cmd();
    ObjectIdentifierRule parse_object_def();

    ////////////////////////////////////////////////////////////////////////////////
    // | INTERNAL HELPERS |
    ////////////////////////////////////////////////////////////////////////////////

   public:
    void fetch_tokens() {
        if (m_EndOfInputReached) {
            return;
        }

        constexpr size_t fetch_count = 16;
        m_Tokens.reserve(m_Tokens.size() + (fetch_count * 2));

        Token tok{};
        for (size_t i = 0; i < fetch_count; ++i) {
            if (!m_Lexer->read_token(&tok)) {
                m_EndOfInputReached = true;
                break;
            }

            if (!tok.is_comment()) {
                m_Tokens.push_back(tok);
            }
        }
        m_Index = 0;
    }

    /**
     * Attempts to advance the stream forward.
     * @note This will attempt to fetch more tokens.
     */
    void advance() {
        if ((m_Index + 1) >= m_Tokens.size()) {
            fetch_tokens();
        }
        m_Index = std::clamp(m_Index + 1, size_t{0}, m_Tokens.size() - 1);
    }

    /**
     * Requires that the next token is any of the provided throwing if none match.
     *
     * @tparam Kinds The token kinds to expect.
     * @tparam CoalesceIdentifiers If true, TokenKind::Identifier will match keyword tokens.
     * @throws std::runtime_error If the current token is none of the provided.
     * @note This will advance the stream forward if no exception is thrown.
     */
    template <TokenKind... Kinds, bool CoalesceIdentifiers = true>
    void require_next() {
        const Token& tok = peek(1);

        // Coalesce + Contains Identifier keyword
        if constexpr (CoalesceIdentifiers && (... || (TokenKind::Identifier == Kinds))) {
            if (tok.is_keyword() || (... || (tok == Kinds))) {
                advance();
                return;
            }
        } else {
            if ((... || (tok == Kinds))) {
                advance();
                return;
            }
        }

        // Failed, build error message and throw
        std::stringstream ss{};  // NOLINT(*-identifier-length)
        ss << "Expecting:";
        for (const auto& kind : {Kinds...}) {
            ss << std::format(" {},", TokenProxy{kind}.as_str());
        }
        ss << std::format(" but got '{}'", tok.kind_as_str());
        throw std::runtime_error{ss.str()};
    }

    /**
     * Peeks at the next token and checks to see if it is any of the provided.
     * @tparam Kinds The token kinds to check for.
     * @tparam CoalesceIdentifiers If true, TokenKind::Identifier will match keyword tokens.
     * @return true if the next token is any of the provided.
     * @note This will advance the stream if true is returned.
     */
    template <TokenKind... Kinds, bool CoalesceIdentifiers = true>
    bool maybe_next() {
        const Token& tok = peek(1);
        if constexpr (CoalesceIdentifiers && (... || (TokenKind::Identifier == Kinds))) {
            if (tok.is_keyword() || (... || (tok == Kinds))) {
                advance();
                return true;
            }
        } else {
            if ((... || (tok == Kinds))) {
                advance();
                return true;
            }
        }
        return false;
    }

    const Token& peek(int offset = 0) {
        size_t index = m_Index + offset;

        // Stream in tokens as needed
        while (index >= m_Tokens.size() && !m_EndOfInputReached) {
            fetch_tokens();
        }

        // EOF met so peek is out of bounds
        if (index >= m_Tokens.size()) {
            return token_invalid;
        }

        // Return the token
        return m_Tokens[index];
    }
};

}  // namespace tm_parse
