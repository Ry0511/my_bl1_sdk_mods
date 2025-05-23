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
                    TXT_LOG(" > {:}", line.substr(0, 50)); // Only show the first 50 chars of each line
                }
                if (err.has_error_line()) {
                    TXT_LOG(" > {}", err.error_line());
                    TXT_LOG(" > {}", err.error_caret());
                }
            }
        }
    }
};

}
