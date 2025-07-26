//
// Date       : 26/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//


#include "pch.h"

#include "text_mod/text_mod_loader.h"
#include "tests/catch.hpp"

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


TEST_CASE("Text Mod Loader") {
    TextModLoader loader{};
    str in = mod_text;
    loader.load_from_str(std::move(in));

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
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests
