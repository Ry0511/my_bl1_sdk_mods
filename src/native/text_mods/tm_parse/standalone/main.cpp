//
// Date       : 16/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#define CATCH_CONFIG_RUNNER
#include "tests/catch.hpp"

#include "common/text_mod_common.h"
#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"

////////////////////////////////////////////////////////////////////////////////
// | MAIN |
////////////////////////////////////////////////////////////////////////////////

using namespace tm_parse;

int main() {

    TXT_LOG("== SYMBOLS =====================================================================");
    for (TokenProxy proxy : SymbolTokenIterator{}) {
        TXT_LOG("  {:>2} -> {}", proxy.as_int(), proxy.as_str());
    }

    TXT_LOG("== KEYWORDS ====================================================================");
    for (TokenProxy proxy : KeywordTokenIterator{}) {
        TXT_LOG("  {:>2} -> {}", proxy.as_int(), proxy.as_str());
    }

    {
        fs::path wpc_dump = fs::current_path() / "wpc_obj_dump_utf-8.txt";
        if (fs::is_regular_file(wpc_dump)) {
            std::wifstream stream{wpc_dump};
            using It = std::istreambuf_iterator<wchar_t>;
            str content = std::wstring{It{stream}, It{}};

            TextModLexer lexer{content};
            TextModParser parser{&lexer};
            ProgramRule program = ProgramRule::create(parser);
        }
    }

    // Will pickup tests in linked source files
    int result = Catch::Session().run();
    TXT_LOG("Lexer test main exited with: {}", result);


    {
        TextModLexer lexer{TXT("set foo.baz:bar my_cool_property[0]")};
        // TextModParser parser{&lexer};
    }

    return 0;
}
