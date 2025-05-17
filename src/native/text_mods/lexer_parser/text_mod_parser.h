//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "text_mod_common.h"
#include "text_mod_lexer.h"

namespace bl1_text_mods {

class TextModParser {
private:
    TextModLexer* m_Lexer{};

public:
    explicit TextModParser(TextModLexer* lexer) : m_Lexer(lexer) {};
    ~TextModParser() = default;

public:
    void parse_string() {
        try {
            std::vector<TokenKind> tokens{};
            Token token{};

            while (m_Lexer->read_token(&token)) {
#ifdef TEXT_MODS_USE_WCHAR
                std::wcout << token.token_as_str() << std::endl;
#else
                std::cout << token.token_as_str() << std::endl;
#endif
            }

            if (token == TOK_EOF) {
                std::wcout << TEXT("Reached end of input") << std::endl;
                return;
            }
        } catch (const std::runtime_error& err) {
            std::cout << "Failed to parse text with error: " << err.what() << std::endl;
        }
    }
};

}
