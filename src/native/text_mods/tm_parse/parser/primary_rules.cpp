//
// Date       : 05/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/primary_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

SetCommandRule SetCommandRule::create(TextModParser& parser) {
    TXT_MOD_ASSERT(parser.peek() == TokenKind::Kw_Set, "logic error");

    SetCommandRule rule{};

    rule.m_TextRegion = parser.peek().TextRegion;
    parser.advance();

    rule.m_Object = ObjectIdentifierRule::create(parser);
    rule.m_Property = PropertyAccessRule::create(parser);
    rule.m_TextRegion.extend(rule.m_Property.text_region());

    return rule;
}

}  // namespace tm_parse::rules
