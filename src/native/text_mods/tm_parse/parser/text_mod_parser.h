//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "common/text_mod_common.h"
#include "lexer/text_mod_lexer.h"

#include "parser/parser_rules.h"
#include "parser/primary_rules.h"

namespace tm_parse {

using namespace rules;

class TextModParser {
   private:
    friend ParserBaseRule;

   private:
    ParserRuleKind m_PrimaryRuleKind;    // Current primary rule i.e., Set Command, Object Definition
    ParserRuleKind m_SecondaryRuleKind;  // Current parent rule kind i.e., ParenExpr
    bool m_EndOfInputReached;
    TextModLexer* m_Lexer;
    std::vector<Token> m_Tokens;
    size_t m_Index;

   public:
    explicit TextModParser(TextModLexer* lexer, bool insert_entry_token = false) noexcept;
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

    str_view text() const noexcept { return m_Lexer != nullptr ? m_Lexer->text() : str_view{}; }

    ////////////////////////////////////////////////////////////////////////////////
    // | PRIMARY OPERATIONS |
    ////////////////////////////////////////////////////////////////////////////////

   public:
    SetCommandRule parse_set_cmd();
    ObjectIdentifierRule parse_object_def();

    ////////////////////////////////////////////////////////////////////////////////
    // | INTERNAL HELPERS |
    ////////////////////////////////////////////////////////////////////////////////

   public:  // This is only for rules which need extra context information
    void set_primary(ParserRuleKind kind) noexcept { m_PrimaryRuleKind = kind; }
    void set_secondary(ParserRuleKind kind) noexcept { m_SecondaryRuleKind = kind; }
    ParserRuleKind primary() const noexcept { return m_PrimaryRuleKind; }
    ParserRuleKind secondary() const noexcept { return m_SecondaryRuleKind; }

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
    }

    /**
     * Attempts to advance the stream forward.
     * @note This will attempt to fetch more tokens.
     */
    void advance() {
        if ((m_Index + 1) >= m_Tokens.size()) {
            fetch_tokens();
        }
        m_Index = std::clamp(m_Index + 1, size_t{0}, m_Tokens.size());
    }

   public:
    template <TokenKind... Kinds>
        requires(sizeof...(Kinds) > 0)
    void require(const int offset = 0, const bool coalesce = true) {
        const Token& token = peek(offset);

        if (coalesce && (... || (TokenKind::Identifier == Kinds))) {
            if (token.is_keyword() || (... || (token == Kinds))) {
                advance();
                return;
            }
        } else {
            if ((... || (token == Kinds))) {
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
        ss << std::format(" but got '{}'", token.kind_as_str());
        throw std::runtime_error{ss.str()};
    }

    template <TokenKind... Kinds>
        requires(sizeof...(Kinds) > 0)
    void require_next(const bool coalesce = true) {
        require<Kinds...>(1, coalesce);
    }

    template <TokenKind... Kinds>
        requires(sizeof...(Kinds) > 0)
    bool maybe(const int offset = 0, const bool coalesce = true) {
        const Token& tok = peek(offset);

        if (coalesce && (... || (TokenKind::Identifier == Kinds))) {
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

    template <TokenKind... Kinds>
        requires(sizeof...(Kinds) > 0)
    bool maybe_next(const bool coalesce = true) {
        return maybe<Kinds...>(1, coalesce);
    }

    Token peek(int offset = 0) {
        size_t index = m_Index + offset;

        // Stream in tokens as needed
        while (index >= m_Tokens.size() && !m_EndOfInputReached) {
            fetch_tokens();
        }

        // EOF met so peek is out of bounds
        if (index >= m_Tokens.size()) {
            return token_eof;
        }

        // Return the token
        return m_Tokens[index];
    }
};

}  // namespace tm_parse
