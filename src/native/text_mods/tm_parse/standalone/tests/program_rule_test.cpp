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

TEST_CASE("Real Data") {
    // Dumped from WillowPlayerController
    str_view test_str = TXT(R"(
      Begin Object Class=Foo Name=Baz
        ShakeOffsetLength=0.000000
        ShakeRotLength=0.000000
        ShakeFOVLength=0.000000
        PresenceUpdateInterval=240
        PresenceStrings(0)=(Description="Character, Level, Health",PresenceMode=1,PresenceContext=EPMC_Any,Max=0)
        PresenceStrings(1)=(Description="Weapon Mfg Data",PresenceMode=2,PresenceContext=EPMC_OnFoot,Max=0)
        PresenceStrings(2)=(Description="Driving",PresenceMode=3,PresenceContext=EPMC_Driving,Max=0)
        PresenceStrings(3)=(Description="Kills",PresenceMode=4,PresenceContext=EPMC_Any,Max=9000)
        PresenceStrings(4)=(Description="Guns found",PresenceMode=5,PresenceContext=EPMC_Any,Max=999999)
        PresenceStrings(5)=(Description="Duels won",PresenceMode=6,PresenceContext=EPMC_Any,Max=999)
        PresenceStrings(6)=(Description="Dueling",PresenceMode=7,PresenceContext=EPMC_Dueling,Max=0)
        PresenceStrings(7)=(Description="Arenaing",PresenceMode=8,PresenceContext=EPMC_Arenaing,Max=0)
        PresenceStrings(8)=(Description="PlaythroughProgress",PresenceMode=9,PresenceContext=EPMC_Any,Max=126)
        PresenceStrings(9)=(Description="TotalProgress",PresenceMode=10,PresenceContext=EPMC_Any,Max=252)
        PresenceStrings(10)=(Description="JoinMyFight",PresenceMode=11,PresenceContext=EPMC_OpenSlots,Max=4)
        CurrentLunge=(Target=None,Length=0.000000,Start=0.000000,SavedAccelRate=0.000000)
        LungeSpeedModifier=None
        MaxLungeDistance=0.000000
        MaxLungeDistanceBaseValue=0.000000
        CurrentLurch=(StartTime=0.000000,LocMagnitude=0.000000,RotMagnitude=0.000000,Duration=0.000000,FalloffRate=0.000000)
      End Object
    )");

    TextModLexer lexer{test_str};
    TextModParser parser{&lexer};

    try {
        ProgramRule rule = ProgramRule::create(parser);
        REQUIRE(rule.operator bool());
        REQUIRE(rule.rules().size() == 1);
    } catch (const std::runtime_error& err) {
        TXT_LOG("Exception parsing program rule: {}", err.what());
        FAIL();
    }
}

TEST_CASE("Array Access Set Commands") {
    // clang-format off
str_view test_case = TXT(R"(
set WillowPlayerController_0 PresenceStrings(0) (Description="Character, Level, Health",PresenceMode=1,PresenceContext=EPMC_Any,Max=0)
set WillowPlayerController_0 PresenceStrings(1) (Description="Weapon Mfg Data",PresenceMode=2,PresenceContext=EPMC_OnFoot,Max=0)
set WillowPlayerController_0 PresenceStrings(2) (Description="Driving",PresenceMode=3,PresenceContext=EPMC_Driving,Max=0)
set WillowPlayerController_0 PresenceStrings(3) (Description="Kills",PresenceMode=4,PresenceContext=EPMC_Any,Max=9000)
set WillowPlayerController_0 PresenceStrings(4) (Description="Guns found",PresenceMode=5,PresenceContext=EPMC_Any,Max=999999)
set WillowPlayerController_0 PresenceStrings(5) (Description="Duels won",PresenceMode=6,PresenceContext=EPMC_Any,Max=999)
set WillowPlayerController_0 PresenceStrings(6) (Description="Dueling",PresenceMode=7,PresenceContext=EPMC_Dueling,Max=0)
set WillowPlayerController_0 PresenceStrings(7) (Description="Arenaing",PresenceMode=8,PresenceContext=EPMC_Arenaing,Max=0)
set WillowPlayerController_0 PresenceStrings(8) (Description="PlaythroughProgress",PresenceMode=9,PresenceContext=EPMC_Any,Max=126)
set WillowPlayerController_0 PresenceStrings(9) (Description="TotalProgress",PresenceMode=10,PresenceContext=EPMC_Any,Max=252)
set WillowPlayerController_0 PresenceStrings(10) (Description="JoinMyFight",PresenceMode=11,PresenceContext=EPMC_OpenSlots,Max=4)
)");
    // clang-format on

    {
        std::stringstream ss{};
        ss << std::format("{}", str{test_case});

        for (std::string line; std::getline(ss, line);) {
            if (line.empty()) {
                continue;
            }
            str test = to_str<str>(line);
            TextModLexer lexer{test};
            TextModParser parser{&lexer};

            SetCommandRule rule = SetCommandRule::create(parser);
            REQUIRE(rule.operator bool());
            REQUIRE(rule.to_string(parser) == test);
            REQUIRE(rule.property().array_access().operator bool());
            REQUIRE(rule.expr().is<ParenExprRule>());
        }
    }

    try {
        TextModLexer lexer{test_case};
        TextModParser parser{&lexer};
        ProgramRule rule = ProgramRule::create(parser);
        REQUIRE(rule.operator bool());
        REQUIRE(rule.rules().size() == 11);
    } catch (const std::exception& err) {
        TST_INFO("Exception: {}", err.what());
        FAIL();
    }
}

}  // namespace tm_parse_tests