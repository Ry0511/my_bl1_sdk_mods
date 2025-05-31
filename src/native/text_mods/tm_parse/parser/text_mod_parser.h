//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "common/text_mod_common.h"
#include "lexer/text_mod_lexer.h"
#include "parser/parser_rules.h"
#include "parser_rules.h"

namespace tm_parse {

using namespace rules;

class TextModParser {
   private:
    friend class SetCommandRule;
    friend class ObjectIdentifierRule;
    friend class DotIdentifierRule;
    friend class ArrayAccessRule;
    friend class PropertyAccessRule;

   public:
    static constexpr size_t dflt_pool_size{1024};

   private:
    TextModLexer* m_Lexer{};
    ProgramRule* m_ProgramRule{};
    std::vector<TokenKind> m_Tokens;
    std::vector<TokenTextView> m_Regions;

    ParserRuleKind m_CurrentRule{};

   public:
    explicit TextModParser(TextModLexer* lexer) : m_Lexer(lexer) {};
    ~TextModParser() = default;

   private:
    size_t size() const noexcept { return m_Tokens.size(); }
    size_t top() const noexcept {
        TXT_MOD_ASSERT(!m_Tokens.empty(), "top() called on empty stack");
        return m_Tokens.size() - 1;
    }

    TokenKind peek(int offset = 0) const {
        TXT_MOD_ASSERT(size() == m_Regions.size(), "unexpected {} != {}", size(), m_Regions.size());
        TXT_MOD_ASSERT((size()) > offset, "peek out of bounds: {}, {}", size(), offset);
        return m_Tokens[(size() - 1) - offset];
    };

    TokenTextView peek_region(int offset = 0) const {
        TXT_MOD_ASSERT(size() == m_Regions.size(), "unexpected {} != {}", size(), m_Regions.size());
        TXT_MOD_ASSERT((size() - 1) > offset, "peek out of bounds: {}, {}", size(), offset);
        return m_Regions[(size() - 1) - offset];
    }

    str_view get_substr_from_index(size_t begin_index, size_t end_index) const noexcept {
        const TokenTextView& begin_region = get_region(begin_index);
        const TokenTextView& end_region = get_region(end_index);
        str_view text = m_Lexer->text();

        return text.substr(begin_region.Start, end_region.end() - begin_region.Start);
    }

    /**
     * Commits the given token to the parser.
     * @param token The token to commit.
     */
    void push_token(const Token& token) noexcept {
        m_Tokens.emplace_back(token.Kind);
        m_Regions.emplace_back(token.TextRegion);
    }

    /**
     * Reads the next token throwing an exception if the token is not the one provided.
     * @param kind The token to expect.
     * @throws std::runtime_error If no token was read or if the token was not the expected.
     */
    void expect(TokenKind kind) {
        Token token{};

        // Skip all comments
        while (m_Lexer->read_token(&token)) {
            if (!token.is_comment()) {
                break;
            }
        }

        // TODO: This is a hack
        if (kind == TokenKind::Identifier && token.is_identifier()) {
            token.Kind = TokenKind::Identifier;
            push_token(token);
            return;
        }

        if (token == kind) {
            push_token(token);
            return;
        }

        throw std::runtime_error(
            std::format(
                "Expected '{}', got '{}' from text '{}'",
                TokenProxy{kind}.as_str(),
                token.kind_as_str(),
                token.to_string()
            )
        );
    }

    /**
     * Saves the lexer state and reads the next token if the next token is the expected it commits
     * the token with `push_token` otherwise it restores the lexer state.
     * @param kind The maybe next token kind.
     * @return True if the given token was next, false otherwise.
     */
    bool maybe_next(TokenKind kind) {
        Token token{};

        // TODO: This save/restore is quite hacky and results in extra lexing which could be
        //        avoided.
        m_Lexer->save();

        while (m_Lexer->read_token(&token)) {
            if (token.is_comment() || token == TokenKind::BlankLine) {
                continue;
            }

            if (token == kind) {
                push_token(token);
                return true;
            }
        }

        m_Lexer->restore();

        return false;
    }

   private:
    const TokenTextView& get_region(size_t index) const noexcept { return m_Regions[index]; }
    TokenKind get_token(size_t index) const noexcept { return m_Tokens[index]; }

   private:
    void parse_internal(const Token& token);

   public:
    DotIdentifierRule parse_dot_identifier();
    ObjectIdentifierRule parse_object_identifier();
    ArrayAccessRule parse_array_access();
    PropertyAccessRule parse_property_access();
    CompositeExprRule parse_composite_expr();
    SetCommandRule parse_set_command();

   public:
    void parse_program();

   public:
    void parse_string() {
        try {
            std::vector<TokenKind> tokens{};
            Token token{};

            while (m_Lexer->read_token(&token)) {
                TXT_LOG("{}", token.token_as_str());
            }

            if (token == token_eof) {
                TXT_LOG("Reached end of input");
                return;
            }
        } catch (const ErrorWithContext& err) {
            TXT_LOG("Failed to parse text: '{}'", err.what());
            if (err.has_context()) {
                for (const auto& line : err.context_lines()) {
                    TXT_LOG(" > {:.50}", line);  // Only show the first 50 chars of each line
                }
                if (err.has_error_line()) {
                    TXT_LOG(" > {}", err.error_line());
                    TXT_LOG(" > {}", err.error_caret());
                }
            }
        }
    }
};

}  // namespace tm_parse
