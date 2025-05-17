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

int main() {
    using namespace bl1_text_mods;
    TextModLexer lexer{test_str};
    TextModParser parser{&lexer};
    parser.parse_string();

    return 0;
}
