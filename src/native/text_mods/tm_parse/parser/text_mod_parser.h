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
    TextModLexer* m_Lexer;
    std::deque<Token> m_TokenPool;
    std::vector<Token> m_Tokens;

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

    ////////////////////////////////////////////////////////////////////////////////
    // | PRIMARY OPERATIONS |
    ////////////////////////////////////////////////////////////////////////////////

   public:


    ////////////////////////////////////////////////////////////////////////////////
    // | INTERNAL HELPERS |
    ////////////////////////////////////////////////////////////////////////////////

   private:

    Token next_token() noexcept(true) {
        if (!m_TokenPool.empty()) {
        }
    }

    /**
     * Consumes the next token on the stream asserting that the token kind is one of the provided.
     * @param kinds The
     */
    template<typename... Kinds>
    void require(const Kinds&&... kinds) noexcept(false) {
        Token token = next_token();
    }

    const Token& peek(int offset) noexcept(true) {
        if (offset >= 0) {
            return get_token(m_Tokens.size() - offset);
        }
        return get_token(0);
    }
};

}  // namespace tm_parse
