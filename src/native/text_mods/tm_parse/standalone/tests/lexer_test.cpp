//
// Date       : 26/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"
#include "tests/catch.hpp"

namespace tm_parse_tests {

using namespace tm_parse;

// NOLINTBEGIN(*-magic-numbers, *-function-cognitive-complexity)

////////////////////////////////////////////////////////////////////////////////
// | SIMPLE TOKEN LEXING TESTS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Simple Token Lexing") {
    SECTION("Lexing known simple tokens") {
        const txt_char* test_str = TXT("().,=:[]");
        TextModLexer lexer{test_str};
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
        const txt_char* test_str = TXT("`{}+");
        TextModLexer lexer{test_str};
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
            REQUIRE(token.as_str_view() == TXT("#"));
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }

        {
            str the_str = TXT("# Single line comment\n");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE((lexer.read_token(&token) && token == TokenKind::LineComment));
            REQUIRE(token.as_str_view() == TXT("# Single line comment"));

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
            REQUIRE(token.as_str_view() == the_str);
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }

        {
            str the_str = TXT("/**/");
            TextModLexer lexer{the_str};
            Token token{};

            REQUIRE((lexer.read_token(&token) && token == TokenKind::MultiLineComment));
            REQUIRE(token.as_str_view() == the_str);
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
            REQUIRE(token.as_str_view() == the_str);
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
        const txt_char* valid_inputs[]{
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
            REQUIRE(token.as_str_view() == the_str);
            REQUIRE((!lexer.read_token(&token) && token == TokenKind::EndOfInput));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// | REAL DATA LEXING |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Lexing real data") {
    using TokenKind::BlankLine;
    using TokenKind::Comma;
    using TokenKind::Equal;
    using TokenKind::Identifier;
    using TokenKind::Kw_Begin;
    using TokenKind::Kw_Class;
    using TokenKind::Kw_Name;
    using TokenKind::Kw_None;
    using TokenKind::Kw_Object;
    using TokenKind::Kw_Set;
    using TokenKind::LeftParen;
    using TokenKind::NameLiteral;
    using TokenKind::Number;
    using TokenKind::RightParen;

    SECTION("WPC Object Dump") {
        const txt_char* utf8_file = TXT("wpc_obj_dump_utf-8.txt");
        fs::path the_file = fs::current_path() / utf8_file;

#ifdef TEXT_MODS_USE_WCHAR
        std::wifstream stream{the_file};
#else
        std::ifstream stream{the_file};
#endif

        // clang-format off
        const TokenKind expected_tokens[] {
            Kw_Begin  ,                                // Begin
            Kw_Object ,                                // Object
            Kw_Class  , Equal, Identifier,             // Class=WillowPlayerController
            Kw_Name   , Equal, Identifier, BlankLine,  // Name=WillowPlayerController_0
            Identifier, Equal, BlankLine ,             // VfTable_IIUpdatePostProcessOverride=
            Identifier, Equal, BlankLine ,             // VfTable_IIPlayerBehavior=
            Identifier, Equal, BlankLine ,             // VfTable_IIPlayerMaster=
            Identifier, Equal, BlankLine ,             // VfTable_IIScreenParticle=
            Identifier, Equal, BlankLine ,             // VfTable_IIInstanceData=
            Identifier, Equal, BlankLine ,             // VfTable_IIResourcePoolOwner=
            Identifier, Equal, BlankLine ,             // VfTable_FCallbackEventDevice=
            Identifier, Equal, Number    , BlankLine,  // WeaponImpulse=600.000000
            Identifier, Equal, Number    , BlankLine,  // HoldDistanceMin=50.000000
            Identifier, Equal, Number    , BlankLine,  // HoldDistanceMax=750.000000
            Identifier, Equal, Number    , BlankLine,  // ThrowImpulse=800.000000
        };
        // clang-format on

        REQUIRE(fs::exists(the_file));
        REQUIRE(fs::is_regular_file(the_file));
        REQUIRE(stream.is_open());

        using It = std::istreambuf_iterator<txt_char>;
        str content{It{stream}, It{}};

        TextModLexer lexer{content};
        Token token{};

        for (TokenKind expected : expected_tokens) {
            bool ok = lexer.read_token(&token);
            REQUIRE(ok);

            if (token != expected) {
                FAIL(
                    std::format(
                        "Expected: {} but got: ( {}, '{}' )",
                        TokenProxy{expected}.as_str(),
                        TokenProxy{token.Kind}.as_str(),
                        token.as_str()
                    )
                );
            }
        }

        const txt_char* existing_identifiers[]{
            TXT("bAlwaysLookDownCamera"),
            TXT("bHideCompassOnHUD"),
            TXT("bMainMenu_SplitScreen"),
            TXT("bReadyForCommit"),
            TXT("ReplicatedCollisionType"),
            TXT("Timers"),
            TXT("bNetDirty"),
            TXT("PSSMLightCheckLocation"),
            TXT("WillowPlayerController_0"),
        };

        size_t found_count = 0;

        for (const txt_char* identifier : existing_identifiers) {
            while (lexer.read_token(&token)) {
                if (!token.is_identifier()) {
                    continue;
                }

                str_view token_str = token.as_str_view();
                if (token_str != identifier) {
                    continue;
                }

                found_count++;
                break;
            }
        }

        REQUIRE(found_count == (sizeof(existing_identifiers) / sizeof(txt_char*)));
    }

    SECTION("Set Commands") {
        auto test_str = str{TXT("set WillowPlayerController_1 DesiredRotation (Pitch=0,Yaw=0,Roll=0)\n")}
                        + str{TXT("set WillowPlayerController_1 PendingTouch None\n")}
                        + str{TXT("set WillowPlayerController_1 MessageClass Class'Engine.LocalMessage'\n")};

        TextModLexer lexer{test_str};
        Token token{};

        // clang-format off
        TokenKind expected_tokens[] = {
            Kw_Set, Identifier, Identifier,

            LeftParen,
            Identifier, Equal, Number, Comma,
            Identifier, Equal, Number, Comma,
            Identifier, Equal, Number,
            RightParen,
            BlankLine,

            Kw_Set, Identifier, Identifier, Kw_None, BlankLine,
            Kw_Set, Identifier, Identifier, Kw_Class, NameLiteral, BlankLine
        };
        // clang-format on

        for (TokenKind expected : expected_tokens) {
            bool ok = lexer.read_token(&token);
            REQUIRE(ok);

            if (token != expected) {
                FAIL(
                    std::format(
                        "Expected: {} but got: ( {}, '{}' )",
                        TokenProxy{expected}.as_str(),
                        TokenProxy{token.Kind}.as_str(),
                        token.as_str()
                    )
                );
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// | EXPRESSIONS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Lexing parenthesis") {
    using T = TokenKind;

    {
        struct TestCase {
            size_t OpenCount;
            size_t CloseCount;
            str TestStr;
        };

        TestCase test_cases[]{
            TestCase{1, 0,       TXT("(")},
            TestCase{0, 1,       TXT(")")},
            TestCase{1, 1,      TXT("()")},
            TestCase{2, 1,     TXT("(()")},
            TestCase{1, 2,     TXT("())")},
            TestCase{2, 2,    TXT("(())")},
            TestCase{3, 3,  TXT("((()))")},
            TestCase{3, 3, TXT("(((1)))")},
        };

        Token first_token = token_invalid;
        Token last_token{};

        for (const auto& test_case : test_cases) {
            TextModLexer lexer{test_case.TestStr};

            Token tk{};
            size_t open_count = 0;
            size_t close_count = 0;

            while (lexer.read_token(&tk)) {
                if (first_token == token_invalid) {
                    first_token = tk;
                }
                last_token = tk;

                if (tk == T::LeftParen) {
                    REQUIRE(tk.to_string() == TXT("("));
                    open_count++;
                } else if (tk == T::RightParen) {
                    REQUIRE(tk.to_string() == TXT(")"));
                    close_count++;
                }
            }

            REQUIRE(open_count == test_case.OpenCount);
            REQUIRE(close_count == test_case.CloseCount);

            TokenTextView vw = first_token.TextRegion;
            vw.extend(last_token.TextRegion);
            REQUIRE(vw.view_from(lexer.text()) == test_case.TestStr);
        }
    }

    {
        TextModLexer lexer{TXT(")))((((()))))(((")};
        TokenKind expected[]{
            T::RightParen,
            T::RightParen,
            T::RightParen,
            T::LeftParen,
            T::LeftParen,
            T::LeftParen,
            T::LeftParen,
            T::LeftParen,
            T::RightParen,
            T::RightParen,
            T::RightParen,
            T::RightParen,
            T::RightParen,
            T::LeftParen,
            T::LeftParen,
            T::LeftParen,
        };
        Token tk{};
        size_t index = 0;

        while (lexer.read_token(&tk)) {
            const TokenKind expect = expected[index++];
            REQUIRE(tk == expect);

            if (expect == T::LeftParen) {
                REQUIRE(tk.to_string() == TXT("("));
            } else if (expect == T::RightParen) {
                REQUIRE(tk.to_string() == TXT(")"));
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// | LEXING PAREN EXPRESSIONS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("should lex valid sequence") {
    using namespace tokens_enum;
    str_view test_case = TXT(
        R"(PresenceStrings(0)=(Description="Character, Level, Health",PresenceMode=1,PresenceContext=EPMC_Any,Max=0))"
    );
    TextModLexer lexer{test_case};
    Token tk{};

    std::vector<TokenKind> expected{
        Identifier,    LeftParen, Number,     RightParen, Equal,  LeftParen,  Identifier, Equal,
        StringLiteral, Comma,     Identifier, Equal,      Number, Comma,      Identifier, Equal,
        Identifier,    Comma,     Identifier, Equal,      Number, RightParen,
    };

    for (TokenKind expect : expected) {
        REQUIRE(lexer.read_token(&tk));
        REQUIRE(tk.Kind == expect);
    }

    REQUIRE(!lexer.read_token(&tk));
    REQUIRE(tk == EndOfInput);
}

////////////////////////////////////////////////////////////////////////////////
// | LEXER METHODS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Lexer Helpers") {
    SECTION("::get_line_start && ::get_line_number") {
        TextModLexer lexer{TXT("\n1\n12\n123\n1234\n\n\n12345")};
        Token token{};

        auto next_token = [&lexer](Token& token) {
            do {
                REQUIRE(lexer.read_token(&token));
            } while (token.is_eolf());
        };

        REQUIRE(lexer.get_line_start(0) == 0);
        REQUIRE(lexer.get_line_number(0) == 1);

        next_token(token);
        REQUIRE(token.as_str_view() == TXT("1"));
        REQUIRE(token.TextRegion.Start == 1);
        REQUIRE(lexer.get_line_start(token.TextRegion) == 0);
        REQUIRE(lexer.get_line_number(token.TextRegion) == 2);

        next_token(token);
        REQUIRE(token.as_str_view() == TXT("12"));
        REQUIRE(lexer.get_line_start(token.TextRegion) == 3);
        REQUIRE(lexer.get_line_number(token.TextRegion) == 3);

        next_token(token);
        REQUIRE(token.as_str_view() == TXT("123"));
        REQUIRE(lexer.get_line_start(token.TextRegion) == 6);
        REQUIRE(lexer.get_line_number(token.TextRegion) == 4);

        next_token(token);
        REQUIRE(token.as_str_view() == TXT("1234"));
        REQUIRE(lexer.get_line_start(token.TextRegion) == 10);
        REQUIRE(lexer.get_line_number(token.TextRegion) == 5);

        next_token(token);
        REQUIRE(token.as_str_view() == TXT("12345"));
        REQUIRE(lexer.get_line_start(token.TextRegion) == 17);
        REQUIRE(lexer.get_line_number(token.TextRegion) == 8);
    }
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests
