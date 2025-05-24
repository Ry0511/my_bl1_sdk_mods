//
// Date       : 16/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "common/text_mod_common.h"
#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"

////////////////////////////////////////////////////////////////////////////////
// | MAIN |
////////////////////////////////////////////////////////////////////////////////

using namespace tm_parse;

int main() {
    fs::path the_file = fs::current_path() / TXT("wpc_obj_dump.txt");

    if (!fs::is_regular_file(the_file)) {
        TXT_LOG("File does not exist: '{}'", the_file.string());
        return -1;
    }

    std::wifstream stream{the_file};

    using It = std::istreambuf_iterator<wchar_t>;
    str content{It{stream}, It{}};

    TextModLexer lexer{content};
    TextModParser parser{&lexer};
    parser.parse_string();

    TXT_LOG("[SYMBOLS]");
    for (TokenProxy proxy : SymbolTokenIterator{}) {
        TXT_LOG("  {:>2} -> {}", proxy.as_int(), proxy.as_str());
    }

    TXT_LOG("[KEYWORDS]");
    for (TokenProxy proxy : KeywordTokenIterator{}) {
        TXT_LOG("  {:>2} -> {}", proxy.as_int(), proxy.as_str());
    }

    return 0;
}
