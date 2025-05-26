//
// Date       : 26/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "lexer/text_mod_lexer.h"

#define CATCH_CONFIG_RUNNER
#include "standalone/catch.hpp"

namespace tm_parse_tests {

using namespace tm_parse;

////////////////////////////////////////////////////////////////////////////////
// | SIMPLE TOKEN LEXING TESTS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Simple Token Lexing") {
    SECTION("Lexing known simple tokens") {
        const txt_char* s = TXT("().,=:[]");
        TextModLexer lexer{s};
        Token token{};

        REQUIRE((lexer.read_token(&token) && token == TokenKind::LeftParen));
        REQUIRE((lexer.read_token(&token) && token == TokenKind::RightParen));
        REQUIRE((lexer.read_token(&token) && token == TokenKind::Dot));
        REQUIRE((lexer.read_token(&token) && token == TokenKind::Comma));
        REQUIRE((lexer.read_token(&token) && token == TokenKind::Equal));
        REQUIRE((lexer.read_token(&token) && token == TokenKind::Colon));
        REQUIRE((lexer.read_token(&token) && token == TokenKind::LeftBracket));
        REQUIRE((lexer.read_token(&token) && token == TokenKind::RightBracket));
        REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
    }

    SECTION("Lexing unknown tokens") {
        const txt_char* s = TXT("`{}+");
        TextModLexer lexer{s};
        Token token{};

        // By default the lexer will throw on unknown tokens since it's useful for debugging but
        //  this isn't the ideal behaviour.

        REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
        lexer.skip(1);

        REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
        lexer.skip(1);

        REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
        lexer.skip(1);

        REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
        lexer.skip(1);

        REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
    }
}

////////////////////////////////////////////////////////////////////////////////
// | STRING & NAME LITERAL TESTS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Lexing String & Name Literals") {
    SECTION("Valid string and name literals") {
        {
            TextModLexer lexer{TXT("\"Valid string literal!\"")};
            Token token{};
            REQUIRE((lexer.read_token(&token) && token == TokenKind::StringLiteral));
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }

        {
            TextModLexer lexer{TXT("'Valid name literal!'")};
            Token token{};
            REQUIRE((lexer.read_token(&token) && token == TokenKind::NameLiteral));
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }

        {
            TextModLexer lexer{TXT("''")};
            Token token{};
            REQUIRE((lexer.read_token(&token) && token == TokenKind::NameLiteral));

            lexer = TextModLexer{TXT("\"\"")};
            token = Token{};
            REQUIRE((lexer.read_token(&token) && token == TokenKind::StringLiteral));
        }
    }

    SECTION("Unterminated string & name literals") {
        {
            str the_str = TXT("\"Unterminated string literal!");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
            REQUIRE(lexer.position() == the_str.size());
        }

        {
            str the_str = TXT("\"Unterminated name literal!");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
            REQUIRE(lexer.position() == the_str.size());
        }
    }

    SECTION("Invalid string & name literals") {
        {
            str the_str = TXT("\"Invalid string \nliteral!\"");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
        }

        {
            str the_str = TXT("'Invalid name \nliteral!'");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// | SINGLE & MULTI-LINE COMMENT TESTS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Lexing single and multi-line comments") {
    SECTION("Single line comments") {
        {
            TextModLexer lexer{TXT("# Single line comment")};
            Token token{};
            REQUIRE((lexer.read_token(&token) && token == TokenKind::LineComment));
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }

        {
            TextModLexer lexer{TXT("#")};
            Token token{};
            REQUIRE((lexer.read_token(&token) && token == TokenKind::LineComment));
            REQUIRE(token.Text == TXT("#"));
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }

        {
            str the_str = TXT("# Single line comment\n");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE((lexer.read_token(&token) && token == TokenKind::LineComment));
            REQUIRE(token.Text == TXT("# Single line comment"));

            // Since we don't include the \n in the line comment it becomes its own token
            REQUIRE((lexer.read_token(&token) && token == TokenKind::BlankLine));
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }
    }

    SECTION("Multi line comments") {
        {
            str the_str = TXT("/* Multi\nline\ncomment\n*/");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE((lexer.read_token(&token) && token == TokenKind::MultiLineComment));
            REQUIRE(token.Text == the_str);
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }

        {
            str the_str = TXT("/**/");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE((lexer.read_token(&token) && token == TokenKind::MultiLineComment));
            REQUIRE(token.Text == the_str);
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }

        {
            str the_str = TXT("/* Multi line comment \n");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// | NUMBER LEXING |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Lexing numbers") {
    // As long as it is: -?\d+(\.\d+)?
    SECTION("Valid lexer numbers") {
        const txt_char* valid_inputs[]{
            TXT("0"),
            TXT("999999999999999999999999999999999999999999"),
            TXT("-999999999999999999999999999999999999999999"),
            TXT("000000000000000000000000000000000000000009"),
            TXT("-000000000000000000000000000000000000000009"),
            TXT("-1"),
            TXT("-1.0"),
            TXT("-0"),  // Mathematicians are shitting themself
            TXT("-0.0"),
            TXT("-001.001"),
        };

        for (const txt_char* input : valid_inputs) {
            str the_str{input};
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE((lexer.read_token(&token) && token == TokenKind::Number));
            REQUIRE(token.Text == the_str);
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }
    }

    SECTION("Invalid lexer numbers") {
        const txt_char* invalid_inputs[]{
            TXT("0."),
            TXT("-"),
            TXT("-1."),
            TXT("0.-1"),
        };

        for (const txt_char* input : invalid_inputs) {
            str the_str{input};
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE_THROWS_AS(lexer.read_token(&token), ErrorWithContext);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// | IDENTIFIER LEXING |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Lexing identifiers") {

    SECTION("Keyword identifiers") {
        for (TokenProxy proxy : KeywordTokenIterator{}) {
            str the_str{proxy.as_str()};
            TextModLexer lexer{the_str};
            Token token{};

            // clang-format off
            REQUIRE((
                lexer.read_token(&token)
                && token == proxy.as_token()
                && token.is_identifier()
                && token.is_keyword()
            ));
            // clang-format on
        }
    }

    SECTION("Regular identifiers") {
        const txt_char* valid_inputs[] {
            TXT("SomeIdentifier"),
            TXT("otherIdentifier"),
            TXT("my_identifier"),
            TXT("my_id12345"),
            TXT("_my_identifier"),
            TXT("__my_identifier"),
        };

        for (const txt_char* input : valid_inputs) {
            str the_str{input};
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE((lexer.read_token(&token) && token == TokenKind::Identifier));
            REQUIRE(token.Text == the_str);
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// | MAIN |
////////////////////////////////////////////////////////////////////////////////

int lexer_test_main() {
    int result = Catch::Session().run();
    TXT_LOG("Lexer test main exited with: {}", result);
    return result;
}

}  // namespace tm_parse_tests
