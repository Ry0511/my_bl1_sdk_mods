//
// Date       : 16/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#define CATCH_CONFIG_RUNNER
#include "standalone/catch.hpp"

#include "common/text_mod_common.h"
#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"

////////////////////////////////////////////////////////////////////////////////
// | MAIN |
////////////////////////////////////////////////////////////////////////////////

using namespace tm_parse;

int main() {

    TXT_LOG("== WPC LEXING ==================================================================");
    const txt_char* utf8_file = TXT("wpc_obj_dump_utf-8.txt");
    const txt_char* utf16le_file = TXT("wpc_obj_dump_utf-16le.txt");
    fs::path the_file = fs::current_path() / utf8_file;

    if (!fs::is_regular_file(the_file)) {
        TXT_LOG("File does not exist: '{}'", the_file.string());
        return -1;
    }

    std::wifstream stream{the_file};

    using It = std::istreambuf_iterator<wchar_t>;
    str content{It{stream}, It{}};

    TextModLexer lexer{content};
    // TextModParser parser{&lexer};

    TXT_LOG("== SYMBOLS =====================================================================");
    for (TokenProxy proxy : SymbolTokenIterator{}) {
        TXT_LOG("  {:>2} -> {}", proxy.as_int(), proxy.as_str());
    }

    TXT_LOG("== KEYWORDS ====================================================================");
    for (TokenProxy proxy : KeywordTokenIterator{}) {
        TXT_LOG("  {:>2} -> {}", proxy.as_int(), proxy.as_str());
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
