//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "common/text_mod_common.h"
#include "lexer/text_mod_lexer.h"

#include "parser/parser_iterator.h"
#include "parser/parser_rules.h"
#include "parser/primary_rules.h"

namespace tm_parse {

using namespace rules;

class TextModParser {
   public:
    struct PeekOptions {
        bool Coalesce = true;
        bool FailOnBlankLine = false;
        bool SkipOnBlankLine = true;
    };

    struct MatchOptions {
        bool Coalesce = true;        // Allow Identifier to match Kw_*
        bool SkipBlankLines = true;  // Skips BlankLine tokens
    };

    class Iterator {
       private:
        TextModParser* m_Parser;
        size_t m_Index;
        bool m_SkipBlankLines;
    };

   private:
    friend ParserBaseRule;
    friend class ParserIterator;
    using T = TokenKind;

   private:
    std::deque<ParserRuleKind> m_RuleStack;
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
    const TextModLexer* lexer() const noexcept { return m_Lexer; }
    const Token& get_token(size_t index) noexcept(true) {
        if (index >= m_Tokens.size()) {
            return token_eof;
        }
        return m_Tokens[index];
    }

    size_t index() const noexcept { return m_Index; }
    bool eof() const noexcept { return m_EndOfInputReached && m_Index >= m_Tokens.size(); }

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

   public:
    ParserRuleKind peek_rule(size_t offset = 0) const noexcept {
        if (m_RuleStack.empty()) {
            return ParserRuleKind::Unknown;
        }

        if (offset >= m_RuleStack.size()) {
            return ParserRuleKind::Unknown;
        }

        return m_RuleStack[m_RuleStack.size() - 1 - offset];
    }

    void push_rule(ParserRuleKind rule) noexcept {
        m_RuleStack.push_back(rule);
    }

    ParserRuleKind pop_rule() noexcept {
        if (m_RuleStack.empty()) {
            return ParserRuleKind::Unknown;
        }
        auto top = m_RuleStack.back();
        m_RuleStack.pop_back();
        return top;
    }

    bool has_rule(ParserRuleKind rule, int limit = std::numeric_limits<int>::max()) const noexcept {
        if (m_RuleStack.empty()) {
            return false;
        }

        if (m_RuleStack.size() == 1) {
            return m_RuleStack.back() == rule;
        }

        auto it = m_RuleStack.end() - 2;

        for (; it != m_RuleStack.begin() && limit > 0; --it) {
            if (*it == rule) {
                return true;
            }
            --limit;
        }
        return false;
    }

   public:
    ParserIterator create_iterator(const MatchOptions& opt = {}) noexcept {
        return ParserIterator{this, m_Index, opt.SkipBlankLines, opt.Coalesce};
    }

    ParserIterator create_iterator(size_t index, const MatchOptions& opt = {}) noexcept {
        return ParserIterator{this, index, opt.SkipBlankLines, opt.Coalesce};
    }

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

    bool fetch_to_fit(size_t index) {
        while (m_Tokens.size() <= index && !m_EndOfInputReached) {
            fetch_tokens();
        }
        return m_Index < m_Tokens.size();
    }

    /**
     * Attempts to advance the stream forward a single token.
     * @note This will attempt to fetch more tokens.
     * @note This will *only* advance one token. The result is that peek(-1) is the previous token.
     */
    void advance() {
        if ((m_Index + 1) >= m_Tokens.size()) {
            fetch_tokens();
        }
        m_Index = std::clamp(m_Index + 1, size_t{0}, m_Tokens.size());
    }

   public:
    const Token& previous() noexcept { return *(--create_iterator()); }

   public:
    template <TokenKind... Sequence>
        requires(sizeof...(Sequence) > 0)
    int match_seq(MatchOptions opt = {}) noexcept {
        if (eof()) {
            return -1;
        }

        int cur_tok_index = 1;
        size_t pos = m_Index;
        for (TokenKind expected_kind : {Sequence...}) {
            // doaweneedamoretokens?
            while (!m_EndOfInputReached && pos >= m_Tokens.size()) {
                fetch_tokens();
            }

            Token tk = get_token(pos);

            // Skip any blank lines if skipping is enabled
            while (opt.SkipBlankLines && tk == T::BlankLine && expected_kind != T::BlankLine) {
                ++pos;
                tk = get_token(pos);
            }

            // Identifiers match all
            if (opt.Coalesce && tk.is_identifier() && expected_kind == T::Identifier) {
                ++cur_tok_index;
                ++pos;
                continue;
            }

            // Unexpected token in bagging area
            if (tk != expected_kind) {
                return cur_tok_index;
            }

            ++cur_tok_index;
            ++pos;
        }

        return 0;
    }

   public:
    template <TokenKind... Kinds>
        requires(sizeof...(Kinds) > 0)
    void require(const int offset = 0, const PeekOptions& opt = {}) {
        auto impl_error = [this](const Token& token) {
            // Failed, build error message and throw
            std::stringstream ss{};  // NOLINT(*-identifier-length)
            ss << "Expecting one of [";
            bool first = true;
            for (const auto& kind : {Kinds...}) {
                if (!first) {
                    ss << ", ";
                }
                first = false;
                ss << std::format("{}", TokenProxy{kind}.as_str());
            }
            ss << std::format("] but got {}", token.kind_as_str());

            TokenTextView vw = token.TextRegion;
            if (vw.is_valid()) {
                size_t line_number = this->m_Lexer->get_line_number(vw);
                vw.Start = this->m_Lexer->get_line_start(vw);
                vw.Length = m_Lexer->get_line_end(vw.Start) - vw.Start;
                ss << std::format("; at line {}\n  > {}\n", line_number, str{vw.view_from(this->m_Lexer->text())});

                // Position Indicator
                std::string indicator(vw.Length, ' ');
                const size_t start = token.TextRegion.Start - vw.Start;
                const size_t length = std::min(vw.Length, token.TextRegion.Length + 1);
                for (size_t i = start; i < length; ++i) {
                    indicator[i] = '^';
                }
                ss << std::format("  > {}", indicator);
            } else {
                ss << "; Current line is invalid";
            }

            std::stringstream rule_stack{};
            rule_stack << "\nRule Stack: ";
            first = true;
            for (auto rule : m_RuleStack) {
                if (!first) {
                    rule_stack << ", ";
                }
                rule_stack << (int)rule;
                first = false;
            }

            ss << rule_stack.str();

            throw std::runtime_error{ss.str()};
        };

        constexpr bool has_lf = (... || (T::BlankLine == Kinds));
        constexpr bool has_identifier = (... || (T::Identifier == Kinds));

        // Not looking for BlankLine so skip it ( or error )
        if constexpr (!has_lf) {
            if (peek() == T::BlankLine && opt.FailOnBlankLine) {
                impl_error(peek());
            } else {
                while (peek() == T::BlankLine && opt.SkipOnBlankLine) {
                    advance();
                }
            }
        }

        const Token& token = peek(offset);

        if constexpr (has_identifier) {
            if ((opt.Coalesce && token.is_identifier()) || (... || (token == Kinds))) {
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
        constexpr bool has_lf = (... || (T::BlankLine == Kinds));
        constexpr bool has_identifier = (... || (T::Identifier == Kinds));

        // Not looking for BlankLine so skip it ( or error )
        if constexpr (!has_lf) {
            if (peek() == T::BlankLine && opt.FailOnBlankLine) {
                return false;
            } else {
                while (peek() == T::BlankLine && opt.SkipOnBlankLine) {
                    advance();
                }
            }
        }

        const Token& token = peek(offset);

        if constexpr (has_identifier) {
            if ((opt.Coalesce && token.is_identifier()) || (... || (token == Kinds))) {
                advance();
                return true;
            }
        } else {
            if ((... || (token == Kinds))) {
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
