//
// Date       : 16/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "text_mod_common.h"
#include "text_mod_lexer.h"
#include "text_mod_parser.h"

////////////////////////////////////////////////////////////////////////////////
// | MAIN |
////////////////////////////////////////////////////////////////////////////////

using namespace tm_parse;

int main() {
    TextModLexer lexer{test_str};
    TextModParser parser{&lexer};
    parser.parse_string();

    return 0;
}
