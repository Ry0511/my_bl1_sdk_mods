//
// Date       : 25/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/text_mod_parser.h"
#include "utils.h"

namespace tm_parse_tests {

using namespace tm_parse;

TEST_CASE("Valid Program Rule") {
    // clang-format off
str program = TXT(R"(

# Simple Set Commands
set foo baz 1
set foo baz (1)
set foo baz (A=10,B=20,C=30,D=(E=40,F=50,G=60))
set foo baz Unquoted String Literal
set foo baz "Quoted String Literal"
set foo baz Class'foo.baz.bar:child'
set foo baz(0) Foo'baz.bar'
set foo baz(1) Baz'bar.foo'
set foo baz[0] Unquoted Literal

set foo Baz'foo.bar:baz' 1
set foo Baz'foo.bar:baz' (1)
set foo Baz'foo.bar:baz' (A=10,B=20,C=30,D=(E=40,F=50,G=60))
set foo Baz'foo.bar:baz' Unquoted String Literal
set foo Baz'foo.bar:baz' "Quoted String Literal"
set foo Baz'foo.bar:baz' Class'foo.baz.bar:child'
set foo Baz'foo.bar:baz' Foo'baz.bar'
set foo Baz'foo.bar:baz' Baz'bar.foo'
set foo Baz'foo.bar:baz' Unquoted Literal

# Simple Object
Begin Object Class=Foo Name=Baz
End Object

# More Complext Object
Begin Object Class=Foo Name=Parent
  Begin Object Class=Baz Name=Child
    A=10
    B=Foo'baz.bar'
  End Object
  A=Foo'baz.bar'
  B=Unquoted Literal
End Object

)");
    // clang-format on

    TextModLexer lexer{program};
    TextModParser parser{&lexer};

    ProgramRule rule = ProgramRule::create(parser);
    REQUIRE(rule.operator bool());
    REQUIRE(rule.rules().size() == 20);

    SECTION("Text Content Matches") {
        std::vector<str> expected{
            TXT("set foo baz 1"),
            TXT("set foo baz (1)"),
            TXT("set foo baz (A=10,B=20,C=30,D=(E=40,F=50,G=60))"),
            TXT("set foo baz Unquoted String Literal"),
            TXT("set foo baz \"Quoted String Literal\""),
            TXT("set foo baz Class'foo.baz.bar:child'"),
            TXT("set foo baz(0) Foo'baz.bar'"),
            TXT("set foo baz(1) Baz'bar.foo'"),
            TXT("set foo baz[0] Unquoted Literal"),
            TXT("set foo Baz'foo.bar:baz' 1"),
            TXT("set foo Baz'foo.bar:baz' (1)"),
            TXT("set foo Baz'foo.bar:baz' (A=10,B=20,C=30,D=(E=40,F=50,G=60))"),
            TXT("set foo Baz'foo.bar:baz' Unquoted String Literal"),
            TXT("set foo Baz'foo.bar:baz' \"Quoted String Literal\""),
            TXT("set foo Baz'foo.bar:baz' Class'foo.baz.bar:child'"),
            TXT("set foo Baz'foo.bar:baz' Foo'baz.bar'"),
            TXT("set foo Baz'foo.bar:baz' Baz'bar.foo'"),
            TXT("set foo Baz'foo.bar:baz' Unquoted Literal")
        };

        for (int i = 0; i < expected.size(); ++i) {

            REQUIRE(rule.has<SetCommandRule>(i));
            const SetCommandRule& cmd = rule.get<SetCommandRule>(i);

            REQUIRE(cmd.operator bool());
            TST_INFO("'{}', '{}'", expected[i], str{cmd.to_string(parser)});
            REQUIRE(cmd.to_string(parser) == expected[i]);
        }

        {
            REQUIRE(rule.has<ObjectDefinitionRule>(18));
            const ObjectDefinitionRule& obj = rule.get<ObjectDefinitionRule>(18);
            REQUIRE(obj.operator bool());
            REQUIRE(obj.child_objects().empty());
            REQUIRE(obj.assignments().empty());
        }

        {
            REQUIRE(rule.has<ObjectDefinitionRule>(19));
            const ObjectDefinitionRule& obj = rule.get<ObjectDefinitionRule>(19);
            REQUIRE(obj.operator bool());

            REQUIRE(obj.child_objects().size() == 1);

            auto child = obj.child_objects()[0];
            REQUIRE(child->assignments().size() == 2);
            REQUIRE(child->assignments()[0].to_string(parser) == TXT("A=10"));
            REQUIRE(child->assignments()[1].to_string(parser) == TXT("B=Foo'baz.bar'"));


            REQUIRE(obj.assignments().size() == 2);
            REQUIRE(obj.assignments()[0].to_string(parser) == TXT("A=Foo'baz.bar'"));
            REQUIRE(obj.assignments()[1].to_string(parser) == TXT("B=Unquoted Literal"));

        }
    }
}

}  // namespace tm_parse_tests