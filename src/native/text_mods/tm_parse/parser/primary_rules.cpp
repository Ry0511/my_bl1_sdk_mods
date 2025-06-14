//
// Date       : 05/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/primary_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

SetCommandRule SetCommandRule::create(TextModParser& parser) {
    // TODO: Since this can error we need to always restore primary
    SetCommandRule rule{};
    auto original = parser.primary();
    parser.set_primary(ParserRuleKind::SetCommand);

    parser.require<TokenKind::Kw_Set>();
    rule.m_TextRegion = parser.peek(-1).TextRegion;

    rule.m_Object = ObjectIdentifierRule::create(parser);
    rule.m_Property = PropertyAccessRule::create(parser);
    rule.m_Expression = ExpressionRule::create(parser);

    TXT_MOD_ASSERT(rule.expr().operator bool(), "invalid expression");
    rule.m_TextRegion.extend(rule.expr().text_region());

    parser.set_primary(original);

    return rule;
}

}  // namespace tm_parse::rules
