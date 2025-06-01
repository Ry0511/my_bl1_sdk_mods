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
        const txt_char* test_cases[]{
            TXT("foo"),
            TXT("foo.baz"),
            TXT("foo.baz.bar"),
            TXT("foo_bar._baz_foo"),
        };

        for (const txt_char* test_case : test_cases) {
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};

            DotIdentifierRule rule = DotIdentifierRule::create(parser);
            REQUIRE((rule && test_case == rule.to_string(parser)));
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
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests
