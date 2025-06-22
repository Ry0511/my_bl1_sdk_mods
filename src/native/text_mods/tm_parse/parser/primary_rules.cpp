//
// Date       : 05/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/primary_rules.h"
#include "text_mod_parser.h"

namespace tm_parse::rules {

using namespace tokens;

SetCommandRule SetCommandRule::create(TextModParser& parser) {
    SetCommandRule rule{};
    auto original = parser.primary();
    parser.set_primary(ParserRuleKind::SetCommand);

    parser.require<TokenKind::Kw_Set>();
    rule.m_TextRegion = parser.peek(-1).TextRegion;

    // TODO: Revise this at some point
    // Class'foo.baz:bar'
    if (parser.match_seq<Identifier, NameLiteral>() == 0) {
        rule.m_Object = NameExprRule::create(parser);
    }
    // foo.baz:bar
    else {
        rule.m_Object = ObjectIdentifierRule::create(parser);
    }

    rule.m_Property = PropertyAccessRule::create(parser);
    rule.m_Expression = ExpressionRule::create(parser);

    TXT_MOD_ASSERT(rule.expr().operator bool(), "invalid expression");
    rule.m_TextRegion.extend(rule.expr().text_region());

    parser.set_primary(original);

    return rule;
}

ObjectDefinitionRule ObjectDefinitionRule::create(TextModParser& parser) {
    /*
     *  Begin Object Class=Foo.Baz.Bar Name=Some.Object.Name:Child.Name
     *    Foo(0)=\n
     *    Baz[0]=(1)
     *    Bar   =(X=10, Y=20)
     *  End Object
     */

    ObjectDefinitionRule rule{};
    parser.require<Kw_Begin>();
    rule.m_TextRegion = parser.previous().TextRegion;
    parser.require<Kw_Object>();

    // Class=Foo.Baz.Bar
    parser.require<Kw_Class>();
    parser.require<Equal>();
    rule.m_Class = DotIdentifierRule::create(parser);

    // Name=Foo.Baz:Bar
    parser.require<Kw_Name>();
    parser.require<Equal>();
    rule.m_Name = ObjectIdentifierRule::create(parser);

    // Skip blank line here
    if (parser.peek() == BlankLine) {
        parser.advance();
    }

    auto is_assignment = [&parser]() -> bool {
        // A    =
        // A(0) =
        // B[0] =
        constexpr TextModParser::MatchOptions opt{.Coalesce = true, .SkipBlankLines = false};
        return parser.match_seq<Identifier, Equal>(opt)
               || parser.match_seq<Identifier, LeftParen, Number, RightParen, Equal>(opt)
               || parser.match_seq<Identifier, LeftBracket, Number, RightBracket, Equal>(opt);
    };

    while (parser.match_seq<Kw_End, Kw_Object>() != 0) {

        if (is_assignment()) {

            auto it = parser.create_iterator({.SkipBlankLines = false});
            while (it != Equal) {
                ++it;
            }

            // A=\n
            if ((++it) == BlankLine) {
                while (parser.peek() != Equal) {
                    parser.advance();
                }
                parser.require<Equal>();
                parser.require<BlankLine>();
            } else {
                rule.m_Assignments.emplace_back(AssignmentExprRule::create(parser));
                auto s = rule.m_Assignments.back().to_string(parser);
                parser.require<BlankLine>();
            }

        } else if (parser.match_seq<Kw_Begin, Kw_Object>() != 0) {
            rule.m_ChildObjects.emplace_back(ObjectDefinitionRule::create(parser));
        }
    }

    parser.require<Kw_End>();
    parser.require<Kw_Object>();

    rule.m_TextRegion.extend(parser.previous().TextRegion);

    return rule;
}

}  // namespace tm_parse::rules
