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

    str bad_ = TXT("#Line Three\nset MyObject MyProperty (A=-)\n");

    TextModLexer lexer{bad_};
    TextModParser parser{&lexer};
    parser.parse_string();

    return 0;
}
