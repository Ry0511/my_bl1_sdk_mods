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

    void push_token(const Token& token) noexcept {
        m_Tokens.emplace_back(token.Kind);
        m_Regions.emplace_back(token.TextRegion);
    }

    void expect(TokenKind kind) {
        Token token{};
        if (!m_Lexer->read_token(&token) || token != kind) {
            throw std::runtime_error(
                std::format(
                    "Expected '{}', got '{}' from text '{}'",
                    TokenProxy{kind}.as_str(),
                    token.kind_as_str(),
                    token.as_str()
                )
            );
        }

        // Save the token
        push_token(token);
    }

    bool maybe_next(TokenKind kind) {
        Token token{};

        m_Lexer->save();

        if (m_Lexer->read_token(&token) && token == kind) {
            push_token(token);
            return true;
        }

        m_Lexer->restore();

        return false;
    }

   private:
    void parse_internal(const Token& token);

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
