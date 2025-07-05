//
// Date       : 30/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"
#include "utils.h"

namespace tm_parse_tests {
using namespace tm_parse;
using namespace tokens;

// NOTE
//  This only validates at a high-level more complex tests might be added in specialised files if
//  there is ever a need.
//

// NOLINTBEGIN(*-magic-numbers, *-function-cognitive-complexity)

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
                TXT("\nfoo.baz.bar"),
                TXT("\n\nfoo_bar._baz_foo"),
                TXT("\n\n\nfoo.baz"),
            };
            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};

                DotIdentifierRule rule = DotIdentifierRule::create(parser);
                TXT_DFLT_INFO(test_case, parser, rule);
                REQUIRE(rule.operator bool());

                auto pos = test_case.find_first_not_of(TXT('\n'));
                REQUIRE(rule.to_string(parser) == test_case.substr(pos));
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
                double actual = rule.value<float>();
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
                TXT("Class'Foo'"),
                TXT("Class'foo._baz:_bar'"),
                TXT("_Some_Class'Foo'"),
                TXT("_Some_Class'Foo.Baz'"),
                TXT("_Some_Class'Foo.Baz:Bar'"),
            };

            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                auto name = NameExprRule::create(parser);
                REQUIRE(name.operator bool());
                REQUIRE(name.to_string(parser) == test_case);
            }
        }

        {
            str_view test_cases[] {
                TXT("Class''"),
                TXT("Class'foo.'"),
                TXT("Class'foo:bar.'"),
                TXT("Class'foo.bar:'"),
                TXT("Class'1.2.3'"),
                TXT("0'foo.baz:bar'"),
                TXT("Foo.'foo.baz:bar'")
            };

            for (str_view test_case : test_cases) {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                REQUIRE_THROWS_AS(NameExprRule::create(parser), std::runtime_error);
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
            TXT("foo baz bar\n"),
            TXT(", baz, foo, bar\n"),
            TXT("baz, foo, bar\n"),
            TXT("baz\n"),
            TXT("foo baz\nbar\n"),
            TXT(", baz, foo,\nbar\n"),
            TXT("baz, foo,\nbar\n"),
            TXT("baz\n"),
        };

        for (str_view test_case : test_cases) {
            TextModLexer lexer{test_case};
            TextModParser parser{&lexer};

            parser.push_rule(ParserRuleKind::RulePrimitiveExpr);
            LiteralExprRule rule = LiteralExprRule::create(parser);
            TST_INFO("Result: '{}'", str{rule.to_string(parser)});

            str expected = str{test_case.substr(0, test_case.find_first_of(TXT("\n")))};

            REQUIRE(rule.operator bool());
            REQUIRE(expected == rule.to_string(parser));
        }
    }

    SECTION("Struct Expression") {
        {
            str_view test_cases[]{
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

        {  // Why would you do this? ¯\_(ツ)_/¯

            auto validate = [](str_view test_case) -> ParenExprRule {
                TextModLexer lexer{test_case};
                TextModParser parser{&lexer};
                TST_INFO("Test: '{}'", str{test_case});
                auto rule = ParenExprRule::create(parser);
                REQUIRE(rule.operator bool());
                REQUIRE(rule.to_string(parser) == test_case);
                return rule;
            };

            auto r = validate(TXT("(((((((((((((((((((((((((((((((1)))))))))))))))))))))))))))))))"));
            REQUIRE(r.inner_most()->is<NumberExprRule>());
            REQUIRE(r.inner_most()->get<PrimitiveExprRule>().is<NumberExprRule>());

            // Doesn't actually hold an inner expression
            r = validate(TXT("()"));
            REQUIRE(r.inner_most() == nullptr);
            REQUIRE(!r.has_expr());

            r = validate(TXT("(())"));
            REQUIRE(r.inner_most() == nullptr);
            REQUIRE(r.has_expr());  // Holds a ParenExprRule
            REQUIRE(r.expr().is<ParenExprRule>());

            // Inner most is a string
            r = validate(TXT("(((\"Lorem ipsum\")))"));
            REQUIRE(r.inner_most()->is<StrExprRule>());
            REQUIRE(r.inner_most()->get<PrimitiveExprRule>().is<StrExprRule>());

            // Inner most is a list of assignments
            r = validate(TXT("((A=,B=,C=(())))"));
            REQUIRE(r.inner_most()->is<AssignmentExprListRule>());
            const auto& list = r.inner_most()->get<AssignmentExprListRule>();
            REQUIRE(list.size() == 3);
            REQUIRE(!list[0].has_expr());
            REQUIRE(!list[1].has_expr());

            // AssignmentExpr ( ParenExpr ( ParenExpr ) )
            REQUIRE(list[2].has_expr());
            REQUIRE(list[2].expr().is<ParenExprRule>());
            REQUIRE(list[2].expr().get<ParenExprRule>().expr().is<ParenExprRule>());
            REQUIRE(list[2].expr().get<ParenExprRule>().inner_most() == nullptr);
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
                TXT("Property_7    = False"),
            };

            for (const str& test_case : test_cases) {
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
            auto validate = [](const std::vector<str>& expected, const str& full_text) {
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
            str_view test_cases[]{
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
            TXT("SomeClass'foo.baz:bar'"),
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

TEST_CASE("Set Command") {
    SECTION("Original Tests") {
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

            REQUIRE(rule.object<ObjectIdentifierRule>().to_string(parser) == TXT("Foo.Baz:Bar"));
            REQUIRE(rule.property().to_string(parser) == TXT("MyProperty (1)"));
            REQUIRE(rule.expr().to_string(parser) == TXT("(1)"));
        }
    }

    SECTION("Object NameLiteralExpr") {
        auto validate = [](str_view object, str_view property, str_view expr, str_view full_test) {
            TextModLexer lexer{full_test};
            TextModParser parser{&lexer};

            TST_INFO("Test: {}", str{full_test});

            auto rule = SetCommandRule::create(parser);
            REQUIRE(rule.operator bool());
            TST_INFO("Result: '{}'", str{rule.to_string(parser)});

            REQUIRE(rule.has_object<NameExprRule>());
            REQUIRE(!rule.has_object<ObjectIdentifierRule>());
            REQUIRE(rule.object<NameExprRule>().to_string(parser) == object);
            REQUIRE(rule.property().to_string(parser) == property);
            REQUIRE(rule.expr().to_string(parser) == expr);
        };

        // clang-format off
        validate( TXT("Class'foo.baz'"),
                  TXT("prop"),
                  TXT("1"),
                  TXT("set Class'foo.baz' prop 1")
                );

        validate( TXT("Foo'foo.baz'"),
                  TXT("prop"),
                  TXT("(((1)))"),
                  TXT("set Foo'foo.baz' prop (((1)))")
                );

        validate( TXT("SomeClass'foo.baz'"),
                  TXT("prop"),
                  TXT("(A=10,B=(),C=(X=10,Y=20))"),
                  TXT("set SomeClass'foo.baz' prop (A=10,B=(),C=(X=10,Y=20))")
                );

        validate( TXT("some_class'foo.baz'"),
                  TXT("prop"),
                  TXT("False"),
                  TXT("set some_class'foo.baz' prop False")
                );

        validate( TXT("_'foo.baz'"),
                  TXT("prop"),
                  TXT("None"),
                  TXT("set _'foo.baz' prop None")
                );

        validate( TXT("_0'foo.baz'"),
                  TXT("prop"),
                  TXT("\"String\""),
                  TXT("set _0'foo.baz' prop \"String\"")
                );
        // clang-format on
    }
}

TEST_CASE("Object Definition") {

    SECTION("Simple Object") {
        str_view test_str = TXT(R"(
        Begin Object Class=Foo.Baz Name=Foo.Baz:Bar
          Property_0    =
          Property_0    = 10
          Property_1    = ()
          Property_2    = ((()))
          Property_3    = (A=10, B=20, C=30)
          Property_4    = "String"
          Property_5    = Class'Foo.Baz.Bar'
          Property_6    = True
          Property_7    = False
          Property_7    = Unquoted True 1 Literal Class () '' 'A'
          Property_8(0) = (1)
          Property_8(1) = (2)
          Property_8(2) = (3)
          Property_8(3) = (4)
          Property_9[0] = "0"
          Property_9[1] = "1"
          Property_9[2] = "2"
          Property_9[3] = "3"
        End Object
        )");

        TextModLexer lexer{test_str};
        TextModParser parser{&lexer};

        auto rule = ObjectDefinitionRule::create(parser);
        REQUIRE(rule.operator bool());
        REQUIRE(rule.child_objects().empty());
        REQUIRE(rule.clazz().to_string(parser) == TXT("Foo.Baz"));
        REQUIRE(rule.name().to_string(parser) == TXT("Foo.Baz:Bar"));

        REQUIRE(rule.assignments().size() == 17);

        // clang-format off
        REQUIRE(rule.assignments()[0].to_string(parser) == TXT(R"(Property_0    = 10)"));
        REQUIRE(rule.assignments()[1].to_string(parser) == TXT(R"(Property_1    = ())"));
        REQUIRE(rule.assignments()[2].to_string(parser) == TXT(R"(Property_2    = ((())))"));
        REQUIRE(rule.assignments()[3].to_string(parser) == TXT(R"(Property_3    = (A=10, B=20, C=30))"));
        REQUIRE(rule.assignments()[4].to_string(parser) == TXT(R"(Property_4    = "String")"));
        REQUIRE(rule.assignments()[5].to_string(parser) == TXT(R"(Property_5    = Class'Foo.Baz.Bar')"));
        REQUIRE(rule.assignments()[6].to_string(parser) == TXT(R"(Property_6    = True)"));
        REQUIRE(rule.assignments()[7].to_string(parser) == TXT(R"(Property_7    = False)"));
        REQUIRE(rule.assignments()[8].to_string(parser) == TXT(R"(Property_7    = Unquoted True 1 Literal Class () '' 'A')"));
        REQUIRE(rule.assignments()[9].to_string(parser) == TXT(R"(Property_8(0) = (1))"));
        REQUIRE(rule.assignments()[10].to_string(parser) == TXT(R"(Property_8(1) = (2))"));
        REQUIRE(rule.assignments()[11].to_string(parser) == TXT(R"(Property_8(2) = (3))"));
        REQUIRE(rule.assignments()[12].to_string(parser) == TXT(R"(Property_8(3) = (4))"));
        REQUIRE(rule.assignments()[13].to_string(parser) == TXT(R"(Property_9[0] = "0")"));
        REQUIRE(rule.assignments()[14].to_string(parser) == TXT(R"(Property_9[1] = "1")"));
        REQUIRE(rule.assignments()[15].to_string(parser) == TXT(R"(Property_9[2] = "2")"));
        REQUIRE(rule.assignments()[16].to_string(parser) == TXT(R"(Property_9[3] = "3")"));
        // clang-format on
    }

    SECTION("Child Objects") {
        str test_case = TXT(R"(
          Begin Object Class=Foo.Baz Name=Outermost
            A=1
            Begin Object Class=Foo Name=Child_0
              B=2
              Begin Object Class=Foo Name=Child_0_0
                C=3
              End Object
            End Object
          End Object
        )");

        {
            TextModLexer lexer{test_case};

            // clang-format off
            std::vector<TokenKind> expected {
                BlankLine, Kw_Begin, Kw_Object,
                Kw_Class, Equal, Identifier, Dot, Identifier,
                Kw_Name, Equal, Identifier, BlankLine,
                Identifier, Equal, Number, BlankLine,
                Kw_Begin, Kw_Object, Kw_Class
            };
            // clang-format on

            Token tk{};
            size_t index = 0;
            while (lexer.read_token(&tk) && index < expected.size()) {
                TokenKind expected_kind = expected[index];  // NOLINT(*-pro-bounds-constant-array-index)
                TST_INFO(
                    "actual={} expecting={} index={}",
                    tk.kind_as_str(),
                    TokenProxy{expected_kind}.as_str(),
                    index
                );
                REQUIRE(tk.Kind == expected_kind);
                ++index;
            }
        }

        TextModLexer lexer{test_case};
        TextModParser parser{&lexer};

        auto rule = ObjectDefinitionRule::create(parser);

        // Outermost
        REQUIRE(rule.operator bool());
        REQUIRE(rule.assignments().size() == 1);
        REQUIRE(rule.child_objects().size() == 1);
        REQUIRE(rule.child_objects().front()->child_objects().size() == 1);
        REQUIRE(rule.child_objects().front()->child_objects().front()->child_objects().empty());

        REQUIRE(rule.assignments().size() == 1);
        REQUIRE(rule.assignments()[0].to_string(parser) == TXT("A=1"));

        {  // Child_0
            const ObjectDefinitionRule& child = *rule.child_objects()[0];
            REQUIRE(child.clazz().to_string(parser) == TXT("Foo"));
            REQUIRE(child.name().to_string(parser) == TXT("Child_0"));
            REQUIRE(child.assignments().size() == 1);
            REQUIRE(child.assignments()[0].to_string(parser) == TXT("B=2"));
        }

        {  // Child_0_0
            const ObjectDefinitionRule& child = *rule.child_objects()[0]->child_objects()[0];
            REQUIRE(child.clazz().to_string(parser) == TXT("Foo"));
            REQUIRE(child.name().to_string(parser) == TXT("Child_0_0"));
            REQUIRE(child.assignments().size() == 1);
            REQUIRE(child.assignments()[0].to_string(parser) == TXT("C=3"));
        }

        { // Strings should be the same
            str the_str = str{rule.to_string(parser)};
            auto pos = test_case.find(TXT("Begin"));
            str trimmed = str{test_case.substr(pos)};
            trimmed = trimmed.substr(0, trimmed.find_last_of(TXT("Object")) + 1);
            REQUIRE(the_str == trimmed);
        }
    }

    SECTION("Malformed Object") {

        str test_case = TXT(R"(
          Begin Object Class=Foo Name=Baz
            Property=
            Property=()
            Property=(INVALID)
            Property=1
            Property=("String")
          End
        )");

        TextModLexer lexer{test_case};
        TextModParser parser{&lexer};

        try {
            auto rule = ObjectDefinitionRule::create(parser);
            FAIL();
        } catch (const std::runtime_error& error) {
            TST_INFO("{}", error.what());
            std::string msg = error.what();
            REQUIRE(msg.find("at line 7"));
        }

    }
}

////////////////////////////////////////////////////////////////////////////////
// | HELPERS |
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Match Sequence") {
    using T = TokenKind;
    str_view test_case = TXT("set\nfoo\n\nbaz\nbar\n10\n\"\"\n20\n");
    TextModLexer lexer{test_case};
    TextModParser parser{&lexer};

    auto match_options = TextModParser::MatchOptions{.Coalesce = false, .SkipBlankLines = true};

    SECTION("Skip blank lines") {
        REQUIRE(
            parser.match_seq<
                T::Kw_Set,
                T::Identifier,
                T::Identifier,
                T::Identifier,
                T::Number,
                T::StringLiteral,
                T::Number,
                T::BlankLine  // Still able to use it to validate
                >(match_options)
            == 0
        );

        // Should Match
        REQUIRE(parser.match_seq<T::Kw_Set>(match_options) == 0);
        REQUIRE(parser.match_seq<T::Kw_Set, T::Identifier>(match_options) == 0);
        REQUIRE(parser.match_seq<T::Kw_Set, T::BlankLine, T::Identifier>(match_options) == 0);
        REQUIRE(parser.match_seq<T::Kw_Set, T::BlankLine, T::Identifier, T::BlankLine>(match_options) == 0);

        // Shouldn't Match
        REQUIRE(parser.match_seq<T::BlankLine>(match_options) == 1);
        REQUIRE(parser.match_seq<T::Kw_Set, T::BlankLine, T::Number>(match_options) == 3);
        REQUIRE(parser.match_seq<T::Kw_Set, T::Number>(match_options) == 2);
        REQUIRE(parser.match_seq<T::Kw_True, T::Number>(match_options) == 1);
    }

    match_options.SkipBlankLines = false;

    SECTION("Dont skip blank lines") {
        // Full Match
        REQUIRE(
            parser.match_seq<
                T::Kw_Set,
                T::BlankLine,
                T::Identifier,
                T::BlankLine,
                T::Identifier,
                T::BlankLine,
                T::Identifier,
                T::BlankLine,
                T::Number,
                T::BlankLine,
                T::StringLiteral,
                T::BlankLine,
                T::Number,
                T::BlankLine>(match_options)
            == 0
        );

        // Should Match
        REQUIRE(parser.match_seq<T::Kw_Set>(match_options) == 0);
        REQUIRE(parser.match_seq<T::Kw_Set, T::BlankLine, T::Identifier, T::BlankLine>(match_options) == 0);
        REQUIRE(parser.match_seq<T::Kw_Set, T::BlankLine>(match_options) == 0);
        REQUIRE(parser.match_seq<T::Kw_Set, T::BlankLine, T::Identifier>(match_options) == 0);

        // Shouldn't Match
        REQUIRE(parser.match_seq<T::Kw_True>(match_options) == 1);
        REQUIRE(parser.match_seq<T::Kw_Set, T::Number>(match_options) == 2);
        REQUIRE(parser.match_seq<T::Kw_Set, T::BlankLine, T::Number>(match_options) == 3);
    }

    SECTION("Coalesce identifiers") {
        match_options.SkipBlankLines = true;
        match_options.Coalesce = true;

        // Should Match
        REQUIRE(parser.match_seq<T::Identifier>(match_options) == 0);
        REQUIRE(parser.match_seq<T::Identifier, T::Identifier>(match_options) == 0);
        REQUIRE(parser.match_seq<T::Identifier, T::Identifier, T::Identifier>(match_options) == 0);

        // Shouldn't Match
        REQUIRE(parser.match_seq<T::Kw_True>(match_options) == 1);
        REQUIRE(parser.match_seq<T::Identifier, T::Kw_Set>(match_options) == 2);
        REQUIRE(parser.match_seq<T::Identifier, T::Identifier, T::BlankLine, T::Number>(match_options) == 4);
    }
}

TEST_CASE("require & maybe") {
    SECTION("require") {
        auto test_str = TXT("set\nFoo.Baz\n:\nBar\nMyProperty");

        {
            TextModLexer lexer{test_str};
            TextModParser parser{&lexer};

            constexpr TextModParser::PeekOptions opt{.SkipOnBlankLine = false};

            REQUIRE_NOTHROW(parser.require<Kw_Set>());
            REQUIRE_NOTHROW(parser.require<BlankLine>());
            REQUIRE_NOTHROW(parser.require<Identifier>());
            REQUIRE_NOTHROW(parser.require<Dot>());
            REQUIRE_NOTHROW(parser.require<Identifier>());
            REQUIRE_NOTHROW(parser.require<BlankLine>());
            REQUIRE_NOTHROW(parser.require<Colon>());
            REQUIRE_NOTHROW(parser.require<BlankLine>());
            REQUIRE_NOTHROW(parser.require<Identifier>());
            REQUIRE_NOTHROW(parser.require<BlankLine>());
            REQUIRE_NOTHROW(parser.require<Identifier>());
            REQUIRE_NOTHROW(parser.require<EndOfInput>());
        }

        {
            TextModLexer lexer{test_str};
            TextModParser parser{&lexer};

            constexpr TextModParser::PeekOptions opt{.SkipOnBlankLine = true};

            REQUIRE_NOTHROW(parser.require<Kw_Set>());
            REQUIRE_NOTHROW(parser.require<Identifier>());
            REQUIRE_NOTHROW(parser.require<Dot>());
            REQUIRE_NOTHROW(parser.require<Identifier>());
            REQUIRE_NOTHROW(parser.require<Colon>());
            REQUIRE_NOTHROW(parser.require<Identifier>());
            REQUIRE_NOTHROW(parser.require<Identifier>());
            REQUIRE_NOTHROW(parser.require<EndOfInput>());
        }

        {
            auto test_str = TXT("set true false none True False None Set");
            TextModLexer lexer{test_str};
            TextModParser parser{&lexer};

            constexpr TextModParser::PeekOptions opt{.Coalesce = true};

            do {
                REQUIRE_NOTHROW(parser.require<Identifier>());
            } while (parser.peek() != EndOfInput);
        }
    }

    SECTION("maybe") {
        auto test_str = TXT("set\nFoo.Baz\n:\nBar\nMyProperty");

        {
            TextModLexer lexer{test_str};
            TextModParser parser{&lexer};

            constexpr TextModParser::PeekOptions opt{.SkipOnBlankLine = false};

            REQUIRE(parser.maybe<Kw_Set>());
            REQUIRE(parser.maybe<BlankLine>());
            REQUIRE(parser.maybe<Identifier>());
            REQUIRE(parser.maybe<Dot>());
            REQUIRE(parser.maybe<Identifier>());
            REQUIRE(parser.maybe<BlankLine>());
            REQUIRE(parser.maybe<Colon>());
            REQUIRE(parser.maybe<BlankLine>());
            REQUIRE(parser.maybe<Identifier>());
            REQUIRE(parser.maybe<BlankLine>());
            REQUIRE(parser.maybe<Identifier>());
            REQUIRE(parser.maybe<EndOfInput>());
        }

        {
            TextModLexer lexer{test_str};
            TextModParser parser{&lexer};

            constexpr TextModParser::PeekOptions opt{.SkipOnBlankLine = true};

            REQUIRE(parser.maybe<Kw_Set>());
            REQUIRE(parser.maybe<Identifier>());
            REQUIRE(parser.maybe<Dot>());
            REQUIRE(parser.maybe<Identifier>());
            REQUIRE(parser.maybe<Colon>());
            REQUIRE(parser.maybe<Identifier>());
            REQUIRE(parser.maybe<Identifier>());
            REQUIRE(parser.maybe<EndOfInput>());
        }

        {
            auto test_str = TXT("set true false none True False None Set");
            TextModLexer lexer{test_str};
            TextModParser parser{&lexer};

            constexpr TextModParser::PeekOptions opt{.Coalesce = true};

            do {
                REQUIRE(parser.maybe<Identifier>());
            } while (parser.peek() != EndOfInput);
        }
    }
}

TEST_CASE("copying to internal buffer") {
    auto test_str = TXT("set Foo.Baz:Bar MyProperty (((A=(),B=1,C=(0.09))))");
    TextModLexer lexer{test_str};
    TextModParser parser{&lexer};

    auto rule = SetCommandRule::create(parser);
    REQUIRE(rule.operator bool());
    REQUIRE(rule.to_string(parser) == test_str);
    REQUIRE(rule.to_string() == str_view{});

    REQUIRE(!rule.has_copy_str());
    rule.copy_str_internal(parser);
    REQUIRE(rule.has_copy_str());
    REQUIRE(rule.to_string() == test_str);
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests
