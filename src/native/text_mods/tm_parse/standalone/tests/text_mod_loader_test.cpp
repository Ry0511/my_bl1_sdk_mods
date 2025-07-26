//
// Date       : 26/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "tests/catch.hpp"
#include "tests/utils.h"
#include "text_mod/text_mod_loader.h"

namespace tm_parse_tests {

using namespace tm_parse;

// NOLINTBEGIN(*-magic-numbers, *-function-cognitive-complexity)

//clang-format off

static str mod_text = TXT(R"(

set foo baz (1)
set foo.baz bar (1)
set foo.baz:bar baz ((1))

set foo _1 "String"
set foo _2 Class'foo.baz:bar'
set foo _3 1
set foo _4 (X="Foo", Y=Class'Foo.Baz:Bar')
set foo _5 Foo Baz Bar

set foo baz 50
set complex foo (X=10,Y=20,Z=30,W=40)
set extra_complex foo (A=(B=(C=(D=(E=50)))))

Begin Object Class=A Name=foo
  _4 = (X="Baz")
End Object

Begin Object Class=A Name=A

  Begin Object Class=B Name=B

    Begin Object Class=C Name=C
      A=10
      B=20
    End Object

    C=30
    D=C'C'
  End Object

  E=B'B'
  F=(A=(B=(C=(D=(E=50)))))

End Object

)");

//clang-format on

TEST_CASE("loading basic set commands") {
    str test_case = TXT(R"(
set foo _1 10
set foo _2 "String"
set foo _3 Foo'Baz'
set foo _4 True
set foo _5 False
set foo _6 None
set foo _7 Literal Expression
)");

    TextModLoader loader{};
    loader.load_from_str(std::move(test_case));
    const NameTable& nt = loader.name_table();

    auto& obj = *loader.begin();
    auto it = obj.begin();

    auto check = [&it, &nt](str_view expected) -> void {
        REQUIRE(it.property().full_name(nt) == expected);
        ++it;
    };

    check(TXT("_1"));
    check(TXT("_2"));
    check(TXT("_3"));
    check(TXT("_4"));
    check(TXT("_5"));
    check(TXT("_6"));
    check(TXT("_7"));
}

TEST_CASE("loading complex set command") {
    str test_case = TXT(R"(
set foo baz (_1=(_2=(_3=(_4=(_5=(A="String", B=10, C=Class'Foo.Baz', D=True))))))
)");

    TextModLoader loader{};
    loader.load_from_str(std::move(test_case));
    const NameTable& nt = loader.name_table();

    auto& obj = *loader.begin();
    auto it = obj.begin();

    auto check = [&it, &nt](str_view expected) -> void {
        REQUIRE(it.property().full_name(nt) == expected);
        ++it;
    };

    check(TXT("baz._1._2._3._4._5.A"));
    check(TXT("baz._1._2._3._4._5.B"));
    check(TXT("baz._1._2._3._4._5.C"));
    check(TXT("baz._1._2._3._4._5.D"));
}

TEST_CASE("overwriting set commands") {
    str test_case = TXT(R"(
set foo _1 1
set foo _1 2
set foo _1 3
set foo _1 4
)");

    TextModLoader loader{};
    loader.load_from_str(std::move(test_case));
    const NameTable& nt = loader.name_table();

    auto& obj = *loader.begin();
    auto it = obj.begin();

    {
        TST_INFO(" > {} = {}", it.property().full_name(nt), str{it->second->to_str(it->first->text())});
        REQUIRE(it.property().full_name(nt) == TXT("_1"));
        REQUIRE(it->second->to_str(it->first->text()) == TXT("4"));
    }

    {
        TST_INFO(" > {} = {}", it.property().full_name(nt), str{it->second->to_str(it->first->text())});
        loader.load_from_str(TXT("set foo _1 2"));
        REQUIRE(it->second->to_str(it->first->text()) == TXT("2"));
    }

    {
        TST_INFO(" > {} = {}", it.property().full_name(nt), str{it->second->to_str(it->first->text())});
        loader.load_from_str(TXT("Begin Object Class=A Name=foo\n _1 = 10 \nEnd Object"));
        REQUIRE(it->second->to_str(it->first->text()) == TXT("10"));
    }
}

TEST_CASE("object definitions") {
    str test_case = TXT(R"(
Begin Object Class=A Name=foo

  Begin Object Class=A Name=baz
    _1 = 20
    _2 = "Inner String"
  End Object

  _1 = 50
  _2 = "String"
  _3 = A'baz'
  _4 = baz.foo.bar

End Object
)");

    TextModLoader loader{};
    loader.load_from_str(std::move(test_case));
    const NameTable& nt = loader.name_table();

    for (auto& obj : loader) {
        str obj_name = str{nt.find(obj.obj_ref())};
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            str full_name = it.property().full_name(nt);
            str enum_name = str{it->second->inner_name()};
            str expr_text = str{it->second->to_str(it->first->text())};

            TXT_LOG("[{:>15}] ( {:>14}, {:12} ) = {}", obj_name, full_name, enum_name, expr_text);
        }
    }

    auto obj = *loader.begin();
    auto it = obj.begin();

    auto check = [&it, &nt](str_view full_name, str_view expr_text) -> void {
        TST_INFO(" > {} = {}", it.property().full_name(nt), str{it->second->to_str(it->first->text())});
        REQUIRE(it.property().full_name(nt) == full_name);
        REQUIRE(it->second->to_str(it->first->text()) == expr_text);
        ++it;
    };

    // Outer Object
    check(TXT("_1"), TXT("50"));
    check(TXT("_2"), TXT("\"String\""));
    check(TXT("_3"), TXT("A'baz'"));
    check(TXT("_4"), TXT("baz.foo.bar"));

    // Inner Object
    obj = *(++loader.begin());
    it = obj.begin();
    check(TXT("_1"), TXT("20"));
    check(TXT("_2"), TXT("\"Inner String\""));
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests
