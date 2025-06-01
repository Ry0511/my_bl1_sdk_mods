//
// Date       : 01/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "tests/catch.hpp"

namespace tm_parse_tests {
using namespace tm_parse;

// NOLINTBEGIN(*-magic-numbers, *-function-cognitive-complexity)

TEST_CASE("TokenTextView") {

    SECTION("Is Valid") {
        TokenTextView invalid{};
        REQUIRE(!invalid.is_valid());

        invalid.Start = 0;
        REQUIRE(!invalid.is_valid());

        invalid = TokenTextView{};
        invalid.Length = 0;
        REQUIRE(!invalid.is_valid());

        auto valid = TokenTextView{};
        valid.Start = 0;
        valid.Length = 0;
        REQUIRE(valid.is_valid());
    }

    SECTION("Equality") {
        TokenTextView left{};
        TokenTextView right{};
        REQUIRE(left == right);

        left.Start = 0;
        REQUIRE(left != right);

        right.Start = 0;
        REQUIRE(left == right);
    }

    SECTION("Extend") {
        str test_str = TXT("My Cool String; My Cool String");
        TokenTextView view_a{0, 2}; // My
        TokenTextView view_b{3, 4}; // Cool
        TokenTextView view_c{8, 6}; // String

        REQUIRE(view_a.view_from(test_str) == TXT("My"));
        REQUIRE(view_b.view_from(test_str) == TXT("Cool"));
        REQUIRE(view_c.view_from(test_str) == TXT("String"));

        view_a.extend(view_b);
        REQUIRE(view_a.view_from(test_str) == TXT("My Cool"));

        view_a.extend(view_c);
        REQUIRE(view_a.view_from(test_str) == TXT("My Cool String"));

        // Extending the same view twice should not affect anything
        view_a.extend(view_c);
        REQUIRE(view_a.view_from(test_str) == TXT("My Cool String"));

        // Down extending results in the same string
        view_a.extend(view_b);
        REQUIRE(view_a.view_from(test_str) == TXT("My Cool String"));

        // Order shouldn't matter
        TokenTextView view_d = view_b;
        view_d.extend(view_a);
        REQUIRE(view_d.view_from(test_str) == TXT("My Cool String"));
    }

}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)
}