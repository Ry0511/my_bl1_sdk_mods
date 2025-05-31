//
// Date       : 30/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"
#include "standalone/catch.hpp"

namespace tm_parse_tests {
using namespace tm_parse;

// NOLINTBEGIN(*-magic-numbers, *-function-cognitive-complexity)

TEST_CASE("[PARSER_RULES] ~ DotIdentifier") {

    { // Simple valid strings
        const str test_data[]{
            TXT("Foo"),
            TXT("Foo.Baz"),
            TXT("Foo.Baz.Bar")
        };

        for (const str& test_str : test_data) {
            TextModLexer lexer{test_str};
            TextModParser parser{&lexer};

            DotIdentifierRule res = parser.parse_dot_identifier();
            REQUIRE(res.operator bool());
            REQUIRE(res.to_string(parser) == test_str);
        }
    }

    { // Partially invalid data
        const str test_data[]{
            TXT(""),
            TXT("0"),
            TXT("-"),
            TXT("Foo.")
            TXT("Foo.Baz.0")
            TXT("_A0._B1.0")
        };

        for (const str& test_str : test_data) {
            TextModLexer lexer{test_str};
            TextModParser parser{&lexer};
            REQUIRE_THROWS_AS(parser.parse_dot_identifier(), std::runtime_error);
        }
    }
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests