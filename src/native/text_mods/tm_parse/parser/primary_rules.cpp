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

    constexpr TextModParser::PeekOptions opt{.Coalesce = true, .SkipOnBlankLine = false};

    ObjectDefinitionRule rule{};
    parser.require<Kw_Begin>(); // Skip blank lines to this
    rule.m_TextRegion = parser.peek(-1).TextRegion;
    parser.require<Kw_Object>(0, opt);

    // Class=Foo.Baz.Bar
    parser.require<Kw_Class>(0, opt);
    parser.require<Equal>(0, opt);
    rule.m_Class = DotIdentifierRule::create(parser);

    // Name=Foo.Baz:Bar
    parser.require<Kw_Name>();
    parser.require<Equal>();
    rule.m_Name = ObjectIdentifierRule::create(parser);

    auto is_assignment = [&parser](ParserIterator& it) -> bool {
        return it.match_seq<Identifier, Equal>() == 0
               || it.match_seq<Identifier, LeftParen, Number, RightParen, Equal>() == 0
               || it.match_seq<Identifier, LeftBracket, Number, RightBracket, Equal>() == 0;
    };

    while (parser.peek() != EndOfInput) {
        auto it = parser.create_iterator();
        if (it == BlankLine) {
            ++it;
        }
        it.set_skip_blank_lines(false);

        if (it.match_seq<Kw_End, Kw_Object>() == 0) {
            break;
        }


        // Recursive child objects
        if (it.match_seq<Kw_Begin, Kw_Object, Kw_Class, Equal, Identifier>() == 0) {
            rule.m_ChildObjects.emplace_back(ObjectDefinitionRule::create(parser));
        }
        // Assignments
        else if (is_assignment(it)) {
            if (it == BlankLine) {
                while (parser.peek() != BlankLine) {
                    parser.advance();
                }
                parser.require<BlankLine>();
            } else {
                rule.m_Assignments.emplace_back(AssignmentExprRule::create(parser));
                parser.require<BlankLine>();
            }
        }
        // Don't know what this is but whatever it is it can go fuck itself
        else {
            std::stringstream ss{};
            auto it = parser.create_iterator();
            ss << "Failed to parse object definition, got sequence: ";

            Token current = *it;
            Token begin{};
            Token end{};

            it.set_skip_blank_lines(false);
            do {
                --it;
            } while (!it->is_any<BlankLine, EndOfInput>());

            if (it == BlankLine) {
                ++it;
            }
            begin = *it;

            do {
                ++it;
            } while (!it->is_any<BlankLine, EndOfInput>());

            if (it == BlankLine) {
                --it;
            }
            end = *it;

            TokenTextView vw = begin.TextRegion;
            vw.extend(end.TextRegion);
            ss << std::format("'{}' current is '{}'", str{vw.view_from(parser.text())}, current.to_string());

            throw std::runtime_error{ss.str()};
        }
    }

    parser.require<Kw_End>();
    parser.require<Kw_Object>();

    rule.m_TextRegion.extend(parser.peek(-1).TextRegion);

    return rule;
}

}  // namespace tm_parse::rules
