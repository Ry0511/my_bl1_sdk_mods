//
// Date       : 05/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/rules/primary_rules.h"
#include "parser/text_mod_parser.h"

namespace tm_parse::rules {

using namespace tokens_enum;
using namespace rules_enum;

SetCommandRule SetCommandRule::create(TextModParser& parser) {
    SetCommandRule rule{};

    parser.push_rule(RuleSetCommand);

    parser.require<TokenKind::Kw_Set>();
    rule.m_TextRegion = parser.peek(-1).TextRegion;

    rule.m_Object = ObjectAccessRule::create(parser);
    rule.m_Property = PropertyAccessRule::create(parser);
    rule.m_Expression = ExpressionRule::create(parser);

    TXT_MOD_ASSERT(rule.expr().operator bool(), "invalid expression");
    rule.m_TextRegion.extend(rule.expr().text_region());

    TXT_MOD_ASSERT(parser.peek_rule() == RuleSetCommand);
    parser.pop_rule();

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

    parser.push_rule(RuleObjectDefinition);

    ObjectDefinitionRule rule{};
    parser.require<Kw_Begin>();  // Skip blank lines to this
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
        size_t index_snapshot = parser.index();

        while (parser.peek() == BlankLine) {
            parser.advance();
        }

        auto it = parser.create_iterator();
        it.set_skip_blank_lines(false);

        // Recursive child objects
        if (it.match_seq<Kw_Begin, Kw_Object, Kw_Class, Equal, Identifier>() == 0) {
            rule.m_ChildObjects.emplace_back(ObjectDefinitionRule::create(parser));
        }
        // Reached terminating sequence
        else if (it.match_seq<Kw_End, Kw_Object>() == 0) {
            break;
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
            Token current_token = parser.peek(0);
            auto it = parser.create_iterator(index_snapshot, {.SkipBlankLines = false});

            ss << "Error parsing Object Definition got sequence:\n  ";

            {
                auto it = parser.create_iterator();
                it.set_skip_blank_lines(false);
                it.set_coalesce(false);
                int count = 5;
                std::deque<Token> tokens{};
                while (count > 0) {
                    if (it == EndOfInput) {
                        --count;
                        continue;
                    }
                    tokens.push_front(*it);
                    --it;
                    --count;
                }

                for (const auto& tk : tokens) {
                    ss << std::format("{}", tk.kind_as_str());
                    if (tk != tokens.back()) {
                        ss << ", ";
                    } else {
                        ss << "\n\n";
                    }
                }
            }

            throw ParsingError::create(ss.str(), parser);
        }
    }

    parser.require<Kw_End>();
    parser.require<Kw_Object>();

    rule.m_TextRegion.extend(parser.peek(-1).TextRegion);
    TXT_MOD_ASSERT(parser.peek_rule() == RuleObjectDefinition);
    parser.pop_rule();

    return rule;
}

ProgramRule ProgramRule::create(TextModParser& parser) {
    ProgramRule rule{};
    bool eof_reached = parser.peek() == EndOfInput;

    parser.push_rule(RuleProgram);

    while (!eof_reached) {
        while (parser.peek() == BlankLine) {
            parser.advance();
        }

        // Set ...
        if (parser.match_seq<Kw_Set>() == 0) {
            rule.m_Rules.emplace_back(SetCommandRule::create(parser));
        }
        // Begin Object ... End Object
        else if (parser.match_seq<Kw_Begin, Kw_Object, Kw_Class, Equal>() == 0) {
            rule.m_Rules.emplace_back(ObjectDefinitionRule::create(parser));
        }
        // EOF token reached
        else if (parser.peek(0) == EndOfInput) {
            eof_reached = true;
        }
        // Unknown/Unsupported
        else {
            // clang-format off
            throw std::runtime_error{
                std::format(
                    "Invalid program rule unknown token '{}'",
                    parser.peek().kind_as_str()
                )
            };
            // clang-format on
        }
    }

    // Update max length text region value
    auto& rules = rule.rules();
    if (!rules.empty()) {
        std::visit(
            [&rule](const auto& inner) -> void { rule.m_MaxLength = inner.text_region().end(); },
            rules[rules.size() - 1]
        );
    }

    TXT_MOD_ASSERT(parser.peek_rule() == RuleProgram);
    parser.pop_rule();

    return rule;
}

}  // namespace tm_parse::rules
