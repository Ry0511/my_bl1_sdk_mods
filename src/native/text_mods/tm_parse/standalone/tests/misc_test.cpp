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

TEST_CASE("validate to_str functions correctly") {

    SECTION("lvalues") {
        std::string hello_regular = "Hello World";
        std::wstring hello_wide = L"Hello World";

        {
            std::string x = to_str<std::string>(hello_regular);
            std::wstring y = to_str<std::wstring>(hello_regular);

            REQUIRE(x == hello_regular);
            REQUIRE(y == hello_wide);
        }

        {
            std::string x = to_str<std::string>(hello_wide);
            std::wstring y = to_str<std::wstring>(hello_wide);

            REQUIRE(x == hello_regular);
            REQUIRE(y == hello_wide);
        }
    }

    SECTION("const lvalues") {
        const std::string hello_regular = "Hello World";
        const std::wstring hello_wide = L"Hello World";

        {
            const std::string& x = to_str<std::string>(hello_regular);
            const std::wstring& y = to_str<std::wstring>(hello_regular);

            REQUIRE(x == hello_regular);
            REQUIRE(y == hello_wide);
        }

        {
            const std::string& x = to_str<std::string>(hello_wide);
            const std::wstring& y = to_str<std::wstring>(hello_wide);

            REQUIRE(x == hello_regular);
            REQUIRE(y == hello_wide);
        }
    }

    SECTION("passing by explicit reference") {
        std::string hello_regular = "Hello World";
        std::wstring hello_wide = L"Hello World";

        { // Basic forwarding as a reference
            std::string x = to_str<std::string>(std::forward<std::string&>(hello_regular));
            std::wstring y = to_str<std::wstring>(std::forward<std::string&>(hello_regular));
            std::string z = to_str<std::string>(std::forward<const std::string&>(hello_regular));
            std::wstring w = to_str<std::wstring>(std::forward<const std::string&>(hello_regular));

            REQUIRE(x == hello_regular);
            REQUIRE(y == hello_wide);
            REQUIRE(z == hello_regular);
            REQUIRE(w == hello_wide);
        }

        // const is required in some places here where a conversion occurs and an rvalue is returned
        // we need to extend the lifetime by binding to an rvalue

        { // Wide -> X
            const std::string& x = to_str<std::string>(hello_wide);
            std::string&& y = to_str<std::string>(hello_wide);
            std::wstring& z = to_str<std::wstring>(hello_wide);

            REQUIRE(x == hello_regular);
            REQUIRE(y == hello_regular);
            REQUIRE(z == hello_wide);
        }

        { // Narrow -> X
            std::string& x = to_str<std::string>(hello_regular);
            const std::wstring& y = to_str<std::wstring>(hello_regular);
            std::wstring&& z = to_str<std::wstring>(hello_regular);

            REQUIRE(x == hello_regular);
            REQUIRE(y == hello_wide);
            REQUIRE(z == hello_wide);
        }
    }

    SECTION("prvalues") {
        REQUIRE(to_str<std::string>(std::string("Hello World")) == "Hello World");
        REQUIRE(to_str<std::wstring>(std::string("Hello World")) == L"Hello World");
        REQUIRE(to_str<std::wstring>(std::wstring(L"Hello World")) == L"Hello World");
        REQUIRE(to_str<std::wstring>(std::wstring(L"Hello World")) == L"Hello World");
    }
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)
}