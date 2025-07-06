//
// Date       : 21/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "parser/text_mod_parser.h"
#include "utils.h"

namespace tm_parse_tests {

using namespace tm_parse;

// NOLINTBEGIN(*-magic-numbers, *-function-cognitive-complexity)

TEST_CASE("TextModParser") {
    using namespace tokens_enum;

    SECTION("Iterator") {
        TextModLexer lexer{TXT("set foo.baz:bar my_cool_property[0]")};
        TextModParser parser{&lexer};

        // Skip BlankLine iterator
        auto iterator = parser.create_iterator({.Coalesce = false, .SkipBlankLines = true});

        TokenKind expected[]{
            Kw_Set,
            Identifier,
            Dot,
            Identifier,
            Colon,
            Identifier,
            Identifier,
            LeftBracket,
            Number,
            RightBracket,
        };

        for (TokenKind kind : expected) {
            TST_INFO("Expecting '{}' == '{}'", TokenProxy{kind}.as_str(), iterator->kind_as_str());
            REQUIRE(iterator == kind);
            ++iterator;
        }

        REQUIRE(iterator == EndOfInput);
        REQUIRE(iterator->is_eolf());
    }

    SECTION("Skipping \\n") {
        TextModLexer lexer{TXT("set\nfoo.baz\n:bar\n\n(1)\n()")};
        TextModParser parser{&lexer};
        auto iterator = parser.create_iterator({.Coalesce = true, .SkipBlankLines = true});

        TokenKind expected[]{
            Kw_Set,
            Identifier,
            Dot,
            Identifier,
            Colon,
            Identifier,
            LeftParen,
            Number,
            RightParen,
            LeftParen,
            RightParen,
        };

        for (TokenKind kind : expected) {
            TST_INFO("Expecting '{}' == '{}'", TokenProxy{kind}.as_str(), iterator->kind_as_str());
            REQUIRE(iterator == kind);
            ++iterator;
        }

        REQUIRE(iterator == EndOfInput);
        REQUIRE(iterator->is_eolf());
    }

    SECTION("Dont skip \\n") {
        TextModLexer lexer{TXT("set\nfoo\n.baz\n\nprop\n1")};
        TextModParser parser{&lexer};
        auto iterator = parser.create_iterator({.SkipBlankLines = false});

        TokenKind expected[]{
            Kw_Set,
            BlankLine,
            Identifier,
            BlankLine,
            Dot,
            Identifier,
            BlankLine,
            Identifier,
            BlankLine,
            Number,
        };

        for (TokenKind kind : expected) {
            TST_INFO("Expecting '{}' == '{}'", TokenProxy{kind}.as_str(), iterator->kind_as_str());
            REQUIRE(iterator == kind);
            ++iterator;
        }

        REQUIRE(iterator == EndOfInput);
        REQUIRE(iterator->is_eolf());
    }

    SECTION("Coalesce identifiers") {
        TextModLexer lexer{TXT("set\ntrue\nfalse none level\nfoo\nbaz\nbar")};
        TextModParser parser{&lexer};
        auto it = parser.create_iterator({.Coalesce = true, .SkipBlankLines = true});

        while (it != EndOfInput) {
            TST_INFO("Token: '{}'", it->as_str());
            REQUIRE(it == Identifier);
            ++it;
        }

        REQUIRE(it == EndOfInput);
        REQUIRE(it->is_eolf());
    }

    SECTION("Dont coalesce identifiers") {
        TextModLexer lexer{TXT("set\ntrue\nfalse none level\nfoo\nbaz\nbar")};
        TextModParser parser{&lexer};
        auto it = parser.create_iterator({.Coalesce = false, .SkipBlankLines = true});

        TokenKind expected[]{
            Kw_Set,
            Kw_True,
            Kw_False,
            Kw_None,
            Kw_Level,
            Identifier,
            Identifier,
            Identifier,
        };

        for (TokenKind kind : expected) {
            TST_INFO("Expecting '{}' == '{}'", TokenProxy{kind}.as_str(), it->kind_as_str());
            REQUIRE(it == kind);

            if (kind != Identifier) {
                REQUIRE(kind != Identifier);
            }

            ++it;
        }

        REQUIRE(it == EndOfInput);
        REQUIRE(it->is_eolf());
    }

    SECTION("Mixed Skipping") {
        TextModLexer lexer{TXT("set\nfoo\n.baz\n\nprop\n1")};
        TextModParser parser{&lexer};
        auto it = parser.create_iterator({.Coalesce = true, .SkipBlankLines = true});

        auto require = [&it](TokenKind kind) {
            TST_INFO("Expecting '{}' == '{}'", TokenProxy{kind}.as_str(), it->kind_as_str());
            REQUIRE(it == kind);
        };

        // clang-format off
        require(Kw_Set);     ++it;
        require(Identifier); ++it;
        require(Dot);        ++it;
        require(Identifier);

        it.set_skip_blank_lines(false);
        ++it;
        require(BlankLine);  ++it;
        require(Identifier); ++it;
        require(BlankLine);  ++it;
        require(Number);     ++it;

        // clang-format on
    }

    SECTION("ParserIterator::operator--") {
        TextModLexer lexer{TXT("set\nfoo\n.baz\n\nprop\n1")};
        TextModParser parser{&lexer};
        auto it = parser.create_iterator({.Coalesce = true, .SkipBlankLines = true});

        auto require = [&it](TokenKind kind) {
            TST_INFO("Expecting '{}' == '{}'", TokenProxy{kind}.as_str(), it->kind_as_str());
            REQUIRE(it == kind);
        };

        // clang-format off

        require(Kw_Set);     ++it;
        require(Identifier); ++it;
        require(Dot);        ++it;
        require(Identifier); ++it;
        require(Identifier); ++it;
        require(Number);     ++it;
        require(EndOfInput);

        --it; require(Number);
        --it; require(Identifier);
        --it; require(Identifier);
        --it; require(Dot);
        --it; require(Identifier);
        --it; require(Kw_Set);

        // --it here does nothing
        --it; require(Kw_Set);

        require(Kw_Set);     ++it;
        require(Identifier); ++it;
        require(Dot);        ++it;
        require(Identifier); ++it;
        require(Identifier); ++it;
        require(Number);     ++it;
        require(EndOfInput);

        // clang-format on
    }

    SECTION("match seq") {
        TextModLexer lexer{TXT("set\nfoo\n.baz\n\nprop\n1")};
        TextModParser parser{&lexer};

        {
            auto it = parser.create_iterator({.Coalesce = true, .SkipBlankLines = true});
            int val = it.match_seq<Kw_Set, Identifier, Dot, Identifier, Identifier, Number>();
            REQUIRE(val == 0);
            REQUIRE(it == EndOfInput);
        }

        {
            auto it = parser.create_iterator({.Coalesce = true, .SkipBlankLines = false});
            int val = it.match_seq<
                Kw_Set,
                BlankLine,
                Identifier,
                BlankLine,
                Dot,
                Identifier,
                BlankLine,
                Identifier,
                BlankLine,
                Number>();
            REQUIRE(val == 0);
            REQUIRE(it == EndOfInput);
        }

        {
            auto it = parser.create_iterator({.Coalesce = true, .SkipBlankLines = true});
            int val = it.match_seq<Kw_Set, Identifier>();
            REQUIRE(val == 0);
            REQUIRE(it == Dot);
        }

        {
            auto it = parser.create_iterator({.Coalesce = true, .SkipBlankLines = true});

            REQUIRE(it.match_seq<Kw_Set, Number>() == 2);
            REQUIRE(it.match_seq<Kw_Set, Identifier, Number>() == 3);
            REQUIRE(it.match_seq<Kw_Set, Identifier, Dot, Number>() == 4);
            REQUIRE(it.match_seq<Kw_Set, Identifier, Dot, Identifier, BlankLine>() == 5);

            {
                it.skip(it.match_seq<Kw_Set, Identifier>());
                TST_INFO("Current: {}", it->kind_as_str());
                REQUIRE(it == Dot);
            }

            {
                it.skip(it.match_seq<Dot, Identifier>());
                TST_INFO("Current: {}", it->kind_as_str());
                REQUIRE(it == Identifier);
            }

            {
                it.set_skip_blank_lines(false);
                it.skip(it.match_seq<Identifier, BlankLine>());
                TST_INFO("Current: {}", it->kind_as_str());
                REQUIRE(it == Number);
            }
        }
    }
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests