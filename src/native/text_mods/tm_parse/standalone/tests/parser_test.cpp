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

#define TST_INFO(fmt, ...) INFO(std::format(fmt, __VA_ARGS__))

#define TXT_DFLT_INFO(test, parser, actual)        \
    TST_INFO(                                      \
        "Rule: ok='{}', to_string='{}'",           \
        actual.operator bool() ? "True" : "False", \
        str{actual.to_string(parser)}              \
    );                                             \
    TST_INFO("Test: in_str='{}'", str{test})

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
                TXT_DFLT_INFO(test_case, parser, rule);
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
                TST_INFO("Should fail: '{}'", str{test_case});
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
            TXT("foo:bar"),
        };

        for (str_view test_case : test_cases) {
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};

            ObjectIdentifierRule rule = ObjectIdentifierRule::create(parser);
            TXT_DFLT_INFO(test_case, parser, rule);
            REQUIRE((rule && rule.primary_identifier() && test_case == rule.to_string(parser)));

            auto pos = test_case.find(TXT(':'));
            if (pos != str_view::npos) {
                str_view primary = test_case.substr(0, pos);
                str_view child = test_case.substr(pos + 1);

                TST_INFO("Primary='{}', Child='{}'", str{primary}, str{child});
                REQUIRE(rule.child_identifier());
                REQUIRE(rule.primary_identifier().to_string(parser) == primary);
                REQUIRE(rule.child_identifier().to_string(parser) == child);
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
                TXT_DFLT_INFO(test_case, parser, rule);
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
                TST_INFO("Should fail: '{}'", str{test_case});
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
            TST_INFO("Test: '{}'", str{test_case});

            PropertyAccessRule rule = PropertyAccessRule::create(parser);
            TXT_DFLT_INFO(test_case, parser, rule);
            REQUIRE((rule && test_case == rule.to_string(parser)));

            if (test_case.find(TXT('(')) != str_view::npos || test_case.find(TXT('[')) != str_view::npos) {
                const ArrayAccessRule& arr_rule = rule.array_access();
                TXT_DFLT_INFO(test_case, parser, arr_rule);
                REQUIRE(rule.array_access());
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// | PRIMITIVE EXPRESSIONS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Expressions") {
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

    SECTION("Struct Expression") {

        {
            str_view test_cases[] {
                TXT("(((A=,B=,C=((1)))))"),
                TXT("()"),
                TXT("(())"),
                TXT("(1)"),
                TXT("((1))"),
                TXT("(INVALID)"),
                TXT("((INVALID))"),
                TXT("(A=,B=,C=((1)))"),
                TXT("((A=,B=,C=((1))))"),
            };

            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                TST_INFO("Test: {}", str{test_case});

                auto rule = ParenExprRule::create(parser);
                REQUIRE(rule.operator bool());
                REQUIRE(rule.to_string(parser) == test_case);
            }
        }

        { // Why would you do this? ¯\_(ツ)_/¯
            str_view test = TXT("(((((((((((((((((((((((((((((((1)))))))))))))))))))))))))))))))");
            TextModLexer lexer{test};
            TextModParser parser{&lexer};
            TST_INFO("Test: {}", str{test});

            auto rule = ParenExprRule::create(parser);
            REQUIRE(rule.operator bool());
            REQUIRE(rule.to_string(parser) == test);
            REQUIRE(rule.inner_most()->get<PrimitiveExprRule>().is<NumberExprRule>());
        }
    }

    SECTION("Assignment Expressions") {
        {
            str test_cases[]{
                TXT("foo=(A=,B=,C=((1)))"),
                TXT("foo = baz"),
                TXT("foo(0) = baz"),
                TXT("foo[0] = baz"),
                TXT("foo=baz"),
                TXT("foo=()"),
                TXT("foo=(())"),
                TXT("foo=((1))"),
            };

            for (str test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                TST_INFO("Test: {}", str{test_case});

                AssignmentExprRule rule = AssignmentExprRule::create(parser);
                REQUIRE(rule.operator bool());

                TST_INFO(" > '{}'", str{rule.to_string(parser)});
                REQUIRE(test_case == rule.to_string(parser));
            }

            {
                auto validate = [](str_view prop, str_view value, str_view full_test) {
                    TextModLexer lexer{full_test};
                    TextModParser parser{&lexer};
                    TST_INFO("Test: '{}'; '{}', '{}'", str{full_test}, str{prop}, str{value});

                    auto rule = AssignmentExprRule::create(parser);
                    REQUIRE(rule.operator bool());

                    TST_INFO(" > '{}'", str{rule.to_string(parser)});
                    TST_INFO(" > '{}'", str{rule.property().to_string(parser)});
                    TST_INFO(" > '{}'", str{rule.expr().to_string(parser)});

                    REQUIRE(rule.to_string(parser) == full_test);
                    REQUIRE(rule.property().to_string(parser) == prop);
                    REQUIRE(rule.expr().to_string(parser) == value);
                };

                // clang-format off
                validate(TXT("foo")   , TXT("1")    , TXT("foo = 1"    ));
                validate(TXT("foo")   , TXT("(1)")  , TXT("foo = (1)"  ));
                validate(TXT("foo")   , TXT("((1))"), TXT("foo = ((1))"));

                validate(TXT("foo(0)"), TXT("1")    , TXT("foo(0) = 1"    ));
                validate(TXT("foo(0)"), TXT("(1)")  , TXT("foo(0) = (1)"  ));
                validate(TXT("foo(0)"), TXT("((1))"), TXT("foo(0) = ((1))"));

                validate(TXT("foo[0]"), TXT("1")    , TXT("foo[0] = 1"    ));
                validate(TXT("foo[0]"), TXT("(1)")  , TXT("foo[0] = (1)"  ));
                validate(TXT("foo[0]"), TXT("((1))"), TXT("foo[0] = ((1))"));
                // clang-format on
            }
        }
    }

    SECTION("Assignment Expression List") {

        {
            auto validate = [](std::vector<str> expected, str full_text) {
                TextModLexer lexer{full_text};
                TextModParser parser{&lexer};
                TST_INFO("Test: {}", str{full_text});
                auto rule = AssignmentExprListRule::create(parser);
                REQUIRE(rule.operator bool());

                size_t index = 0;
                for (const str& expect : expected) {
                    const auto& assign = rule[index];
                    REQUIRE(assign.operator bool());
                    TST_INFO(" > '{}'", index);
                    TST_INFO(" > '{}'", str{assign.property().to_string(parser)});

                    if (assign.has_expr()) {
                        TST_INFO(" > '{}'", str{assign.expr().to_string(parser)});
                    }

                    REQUIRE(assign.to_string(parser) == expect);
                    index++;
                }

                REQUIRE(rule.to_string(parser) == full_text);
            };

            validate({TXT("A=1"), TXT("B=2"), TXT("C=3")}, TXT("A=1,B=2,C=3"));
            validate({TXT("A(0)=1"), TXT("B[1]=2"), TXT("C(2)=3")}, TXT("A(0)=1,B[1]=2,C(2)=3"));
            validate({TXT("A="), TXT("B="), TXT("C=")}, TXT("A=,B=,C="));
            validate({TXT("A=(1)"), TXT("B=(2)"), TXT("C=(3)")}, TXT("A=(1),B=(2),C=(3)"));
        }

        {

            str_view test_cases[] {
                TXT("foo=1,baz=2,bar=3"),
                TXT("foo(1)=1,baz[2]=2,bar=3"),
                TXT("foo=,baz=,bar="),
                TXT("foo=(),baz=(),bar=()"),
                TXT("foo=(()),baz=(()),bar=(())"),
                TXT("foo=((1)),baz=((2)),bar=((3))"),
                TXT("foo=(A=1,B=2),baz=((A=1,B=2)),bar=(((A=1,B=2)))"),
            };

            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};

                TST_INFO("Test: {}", str{test_case});
                auto rule = AssignmentExprListRule::create(parser);

                REQUIRE(rule.operator bool());
                REQUIRE(rule.size() > 1);

                for (const auto& expr : rule) {
                    REQUIRE(expr.operator bool());
                }

                TST_INFO(" > '{}'", str{rule.to_string(parser)});
                REQUIRE(test_case == rule.to_string(parser));
            }
        }
    }

    SECTION("Primitive Expression") {
        str_view test_cases[]{
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
    SECTION("Set Command") {

        {
            str_view test = TXT("set Foo.Baz:Bar MyProperty ((((1))))");
            TextModLexer lexer{test};
            TextModParser parser{&lexer};

            TST_INFO(" > {}", str{test});
            auto rule = SetCommandRule::create(parser);

            REQUIRE(rule.operator bool());
            TST_INFO(" > {}", str{rule.to_string(parser)});
            REQUIRE(test == rule.to_string(parser));
        }

        auto prim_expr = [](str_view in_str) {
            TextModLexer lexer{in_str};
            TextModParser parser{&lexer};
            return SetCommandRule::create(parser).expr().get<PrimitiveExprRule>();
        };

        // Relying on the lifetime being extended to the full expression here because I am lazy
        REQUIRE((prim_expr(TXT("set Foo.Baz:Bar MyProperty 10")).is<NumberExprRule>()));
        REQUIRE((prim_expr(TXT("set Foo.Baz:Bar MyProperty True")).is<KeywordRule>()));
        REQUIRE((prim_expr(TXT("set Foo.Baz:Bar MyProperty False")).is<KeywordRule>()));
        REQUIRE((prim_expr(TXT("set Foo.Baz:Bar MyProperty None")).is<KeywordRule>()));
        REQUIRE((prim_expr(TXT("set Foo.Baz:Bar MyProperty \"My String Literal\"")).is<StrExprRule>()));

        str_view test_cases[]{
            TXT("set Foo.Baz:Bar MyProperty 10"),
            TXT("set Foo.Baz:Bar MyProperty True"),
            TXT("set Foo.Baz:Bar MyProperty False"),
            TXT("set Foo.Baz:Bar MyProperty None"),
            TXT("set Foo.Baz:Bar MyProperty \"My String Literal\""),
            TXT("set Foo.Baz:Bar MyProperty Class'Foo.Baz.Bar'"),
            TXT("set Foo.Baz:Bar MyProperty ()"),
            TXT("set Foo.Baz:Bar MyProperty(1) ()"),
            TXT("set Foo.Baz:Bar MyProperty (1) (1)"),
            TXT("set Foo.Baz:Bar MyProperty (INVALID)"),
            TXT("set Foo.Baz:Bar MyProperty ( X=10, Y=(A=5,B=5,C=5), Z= )"),
            TXT("set Foo.Baz:Bar MyProperty (())"),
            TXT("set Foo.Baz:Bar MyProperty ((()))"),
            TXT("set Foo.Baz:Bar MyProperty ((((1))))"),
        };

        for (str_view test_case : test_cases) {
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};
            TST_INFO("Test: '{}'", str{test_case});
            auto rule = SetCommandRule::create(parser);

            {
                TXT_DFLT_INFO(test_case, parser, rule);
                REQUIRE((rule && test_case == rule.to_string(parser)));
            }
        }

        {
            str_view test_case = TXT("set Foo.Baz:Bar MyProperty (1) (1)");
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};
            TST_INFO("Test: '{}'", str{test_case});
            auto rule = SetCommandRule::create(parser);

            REQUIRE(rule.object().to_string(parser) == TXT("Foo.Baz:Bar"));
            REQUIRE(rule.property().to_string(parser) == TXT("MyProperty (1)"));
            REQUIRE(rule.expr().to_string(parser) == TXT("(1)"));
        }
    }
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests
