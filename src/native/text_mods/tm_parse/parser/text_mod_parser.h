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
   public:
    struct PeekOptions {
        bool Coalesce = true;
        bool FailOnBlankLine = false;
    };

   private:
    friend ParserBaseRule;

   private:
    ParserRuleKind m_PrimaryRuleKind;    // Current primary rule i.e., Set Command, Object Definition
    ParserRuleKind m_SecondaryRuleKind;  // Current parent rule kind i.e., ParenExpr
    bool m_EndOfInputReached;
    TextModLexer* m_Lexer;

    // Contains all tokens
    std::vector<Token> m_Tokens;
    size_t m_Index;

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
    void require(const int offset = 0, const PeekOptions& opt = {}) {
        auto impl_error = [](const Token& token) {
            // Failed, build error message and throw
            std::stringstream ss{};  // NOLINT(*-identifier-length)
            ss << "Expecting:";
            for (const auto& kind : {Kinds...}) {
                ss << std::format(" {},", TokenProxy{kind}.as_str());
            }
            ss << std::format(" but got '{}'", token.kind_as_str());
            throw std::runtime_error{ss.str()};
        };

        // Not looking for BlankLine or EOF?
        if constexpr ((... && (Kinds != TokenKind::BlankLine && Kinds != TokenKind::EndOfInput))) {
            const Token& tk = peek();
            bool is_eof_or_lf = tk.is_eolf();

            if (is_eof_or_lf) {
                if (opt.FailOnBlankLine) {
                    impl_error(tk);
                } else {
                    advance();
                }
            }
        }

        const Token& token = peek(offset);

        if (opt.Coalesce && (... || (TokenKind::Identifier == Kinds))) {
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

        impl_error(token);
    }

    template <TokenKind... Kinds>
        requires(sizeof...(Kinds) > 0)
    void require_next(const PeekOptions& opt = {}) {
        require<Kinds...>(1, opt);
    }

    template <TokenKind... Kinds>
        requires(sizeof...(Kinds) > 0)
    bool maybe(const int offset = 0, const PeekOptions& opt = {}) {
        // If not looking for \n or EOF then skip them
        if constexpr ((... && (Kinds != TokenKind::BlankLine && Kinds != TokenKind::EndOfInput))) {
            const Token& tk = peek();
            bool is_eof_or_lf = tk.is_eolf();

            if (is_eof_or_lf) {
                if (opt.FailOnBlankLine) {
                    return false;
                }
                advance();
            }
        }

        // If not looking for \n or EOF then skip them
        if constexpr ((... && (Kinds != TokenKind::BlankLine && Kinds != TokenKind::EndOfInput))) {
            // TokenKind::BlankLine is greedy and EOF is special
            if (peek().is_eolf()) {
                advance();  // TODO: Might want to enable a flag to error rather than skip for some rules
            }
        }

        const Token& tok = peek(offset);

        if (opt.Coalesce && (... || (TokenKind::Identifier == Kinds))) {
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
    bool maybe_next(const PeekOptions& opt = {}) {
        return maybe<Kinds...>(1, opt);
    }

    /**
     * Retrieves a token at the specified offset.
     * @param offset The offset index from the current position
     * @return The token at the offset found offset, or token_eof if beyond the stream.
     * @note This may attempt to fetch more tokens
     * @note The returned token reference can be invalidated by most methods on the parser i.e.,
     *       advance() or peek()
     */
    const Token& peek(int offset = 0) {
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
