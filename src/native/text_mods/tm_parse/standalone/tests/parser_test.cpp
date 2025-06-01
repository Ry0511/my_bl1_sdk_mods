//
// Date       : 30/05/2025
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

TEST_CASE("Parser Rules") {

    SECTION("Dot Identifier") {
        {
            str_view test_cases[]{
                TXT("foo"),
                TXT("foo.baz"),
                TXT("foo.baz.bar"),
                TXT("foo_bar._baz_foo"),
            };

            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};

                DotIdentifierRule rule = DotIdentifierRule::create(parser);
                REQUIRE((rule && test_case == rule.to_string(parser)));
            }
        }

        {
            const str_view test_cases[]{
                TXT("foo."),
                TXT("foo.baz."),
                TXT("foo.baz.bar."),
            };

            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                REQUIRE_THROWS_AS(DotIdentifierRule::create(parser), std::runtime_error);
            }
        }
    }

    SECTION("Object Identifier") {
        str_view test_cases[]{
            TXT("foo"),
            TXT("foo.baz"),
            TXT("foo.baz.bar"),
            TXT("foo.baz:bar"),
            TXT("foo.baz:bar.baz"),
            TXT("foo.baz:bar.baz.foo"),
        };

        for (str_view test_case : test_cases) {
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};

            ObjectIdentifierRule rule = ObjectIdentifierRule::create(parser);
            REQUIRE((rule && rule.primary_identifier() && test_case == rule.to_string(parser)));

            if (test_case.find(TXT(':')) != str_view::npos) {
                REQUIRE(rule.child_identifier());
            }
        }
    }

    SECTION("Array Access") {
        str_view test_cases[]{
            TXT("[0]"),
            TXT("[-10]"),
            TXT("[99999999999999999999]"),
            TXT("[-99999999999999999999]"),
            TXT("(0)"),
            TXT("(-10)"),
            TXT("(99999999999999999999)"),
            TXT("(-99999999999999999999)"),
        };

        for (str_view test_case : test_cases) {
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};

            auto rule = ArrayAccessRule::create(parser);
            REQUIRE((rule && test_case == rule.to_string(parser)));
        }
    }

    SECTION("Property Access") {
        str_view test_cases[]{
            TXT("foo"),
            TXT("baz_foo"),
            TXT("foo(1)"),
            TXT("foo(0)"),
            TXT("foo[1]"),
            TXT("foo[0]"),
            TXT("foo[0]"),
        };

        for (str_view test_case : test_cases) {
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};

            PropertyAccessRule rule = PropertyAccessRule::create(parser);
            REQUIRE((rule && test_case == rule.to_string(parser)));

            if (test_case.find(TXT('(')) != str_view::npos || test_case.find(TXT('[')) != str_view::npos) {
                REQUIRE(rule.array_access());
            }
        }
    }
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests
