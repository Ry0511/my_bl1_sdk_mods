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

////////////////////////////////////////////////////////////////////////////////
// | COMMON RULES |
////////////////////////////////////////////////////////////////////////////////

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
        {
            str_view test_cases[]{
                TXT("[0]"),
                TXT("[-10]"),
                TXT("(0)"),
                TXT("(-10)"),
            };

            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};

                auto rule = ArrayAccessRule::create(parser);
                REQUIRE((rule && test_case == rule.to_string(parser)));
            }
        }

        {
            str_view test_cases[]{
                TXT("[99999999999999999999]"),
                TXT("[-99999999999999999999]"),
                TXT("(99999999999999999999)"),
                TXT("(-99999999999999999999)"),
            };

            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                REQUIRE_THROWS_AS(ArrayAccessRule::create(parser), std::runtime_error);
            }
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

////////////////////////////////////////////////////////////////////////////////
// | PRIMITIVE EXPRESSIONS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Primitive Expressions") {
    SECTION("Numbers") {
        {
            constexpr float min = std::numeric_limits<float>::min();
            constexpr float max = std::numeric_limits<float>::max();
            auto min_str = std::to_wstring(min);
            auto max_str = std::to_wstring(max);
            std::tuple<str_view, float> test_cases[]{
                {     TXT("123"),    123.0F},
                { TXT("123.456"),  123.456F},
                {    TXT("-123"),   -123.0F},
                {TXT("-123.456"), -123.456F},
                {        min_str,       min},
                {        max_str,       max},
            };

            for (const auto& [test_case, val] : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                NumberExprRule rule = NumberExprRule::create(parser);

                auto s = rule.to_string(parser);
                REQUIRE((rule && test_case == rule.to_string(parser)));

                constexpr double epsilon = std::numeric_limits<float>::epsilon();
                double actual = rule.get<float>();
                REQUIRE(std::fabs(actual - val) < epsilon);
            }
        }  // namespace tm_parse_tests

        {
            str_view test_cases[]{
                TXT("-99999999999999999999999999999999999"),
                TXT("99999999999999999999999999999999999"),
            };
            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                REQUIRE_THROWS_AS(NumberExprRule::create(parser), std::runtime_error);
            }
        }
    }

    SECTION("Name Literals") {
        {
            str_view test_cases[]{
                TXT("Class'Foo.Baz.Bar'"),
                TXT("SomeClass''"),
                TXT("_Some_Class''"),
                TXT("_Some_Class'Some Object Name'"),
            };

            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                auto name = NameExprRule::create(parser);
                REQUIRE((name && name.to_string(parser) == test_case));
            }
        }
    }

    SECTION("Keyword Literals") {
        for (auto proxy : KeywordTokenIterator{}) {
            auto test_str = proxy.as_str();
            TextModLexer lexer{test_str};
            TextModParser parser{&lexer};
            KeywordRule rule = KeywordRule::create(parser);
            REQUIRE((rule && test_str == rule.to_string(parser)));
        }
    }

    SECTION("Literals") {
        str_view test_cases[]{
            TXT("foo baz bar"),
            TXT(", baz, foo, bar"),
            TXT("baz, foo, bar"),
            TXT("baz"),
        };

        for (str_view test_case : test_cases) {
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};
            parser.set_secondary(ParserRuleKind::PrimitiveExpr);
            LiteralExprRule rule = LiteralExprRule::create(parser);
            auto test = rule.to_string(parser);
            REQUIRE((rule && test_case == rule.to_string(parser)));
        }
    }

    SECTION("Primitive Expression") {
        str_view test_cases[] {
            TXT("-3.14"),
            TXT("True"),
            TXT("False"),
            TXT("None"),
            TXT("\"String Literal\""),
            TXT("\"\""),
            TXT("SomeClass'SomeName'"),
            TXT("SomeClass''"),
            TXT("Unquoted String Literal"),
            TXT("AnyLiteral"),
        };

        for (str_view test_case : test_cases) {
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};
            auto rule = PrimitiveExprRule::create(parser);
            REQUIRE((rule && rule.to_string(parser) == test_case));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// | PRIMARY RULES |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Primary Rules") {
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests
