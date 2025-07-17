//
// Date       : 25/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

namespace tm_parse {

using parser_rule_int = uint8_t;

// clang-format off
enum class ParserRuleKind : parser_rule_int {
    RuleIdentifier        ,
    RuleDotIdentifier     ,
    RuleObjectIdentifier  ,
    RuleObjectAccess      ,
    RuleArrayAccess       ,
    RulePropertyAccess    ,
    RuleNumberExpr        ,
    RuleStrExpr           ,
    RuleNameExpr          ,
    RuleKeyword           ,
    RuleLiteralExpr       ,
    RulePrimitiveExpr     ,
    RuleAssignmentExpr    ,
    RuleAssignmentExprList,
    RuleParenExpr         ,
    RuleExpression        ,
    RuleSetCommand        ,
    RuleObjectDefinition  ,
    RuleProgram           ,
    RuleUnknown           ,
};
// clang-format on

constexpr static size_t start_rule_index = static_cast<size_t>(ParserRuleKind::RuleIdentifier);
constexpr static size_t end_rule_index = static_cast<size_t>(ParserRuleKind::RuleUnknown);

// clang-format off
constexpr std::array<str_view, end_rule_index+1> rule_names {
    TXT("Identifier")        ,
    TXT("DotIdentifier")     ,
    TXT("ObjectIdentifier")  ,
    TXT("ObjectAccess")      ,
    TXT("ArrayAccess")       ,
    TXT("PropertyAccess")    ,
    TXT("NumberExpr")        ,
    TXT("StrExpr")           ,
    TXT("NameExpr")          ,
    TXT("Keyword")           ,
    TXT("LiteralExpr")       ,
    TXT("PrimitiveExpr")     ,
    TXT("AssignmentExpr")    ,
    TXT("AssignmentExprList"),
    TXT("ParenExpr")         ,
    TXT("Expression")        ,
    TXT("SetCommand")        ,
    TXT("ObjectDefinition")  ,
    TXT("Program")           ,
    TXT("Unknown")           ,
};
// clang-format on

constexpr static str_view rule_name(ParserRuleKind kind) {
    return rule_names[static_cast<size_t>(kind)];
}

namespace rules_enum {
using ParserRuleKind::RuleArrayAccess;
using ParserRuleKind::RuleAssignmentExpr;
using ParserRuleKind::RuleAssignmentExprList;
using ParserRuleKind::RuleDotIdentifier;
using ParserRuleKind::RuleExpression;
using ParserRuleKind::RuleIdentifier;
using ParserRuleKind::RuleKeyword;
using ParserRuleKind::RuleLiteralExpr;
using ParserRuleKind::RuleNameExpr;
using ParserRuleKind::RuleNumberExpr;
using ParserRuleKind::RuleObjectAccess;
using ParserRuleKind::RuleObjectDefinition;
using ParserRuleKind::RuleObjectIdentifier;
using ParserRuleKind::RuleParenExpr;
using ParserRuleKind::RulePrimitiveExpr;
using ParserRuleKind::RuleProgram;
using ParserRuleKind::RulePropertyAccess;
using ParserRuleKind::RuleSetCommand;
using ParserRuleKind::RuleStrExpr;
using ParserRuleKind::RuleUnknown;
}  // namespace rules_enum

////////////////////////////////////////////////////////////////////////////////
// | GENERIC RULE SETUP |
////////////////////////////////////////////////////////////////////////////////

class TextModLexer;
class TextModParser;

namespace rules {

class ParserBaseRule {
   protected:
    TokenTextView m_TextRegion;
    std::shared_ptr<str> m_Text;

   public:
    const TokenTextView& text_region() const noexcept(true) { return m_TextRegion; }
    operator bool() const noexcept(true) { return m_TextRegion.is_valid(); }

   public:
    str_view to_string(TextModParser& parser) const;
    str_view to_string() const noexcept {
        if (m_Text == nullptr) {
            return str_view{};
        }
        return *m_Text;
    }

    void copy_str_internal(TextModParser& parser);
    bool has_copy_str() const noexcept { return m_Text != nullptr; }

   public:
    void set_text_region(const TokenTextView& text_region) {
        m_TextRegion = text_region;
    }
};

// Not sure if this is actually a good idea or even useful
class ParserPrimaryRule : public ParserBaseRule {
   protected:
    TextModParser* m_Parser{nullptr};
};

#define RULE_PUBLIC_API(type, kind)                     \
                                                        \
   public:                                              \
    void append_tree(strstream& ss, int& indent) const; \
                                                        \
   public:                                              \
    constexpr type() noexcept(true) = default;          \
    constexpr ~type() noexcept(true) = default;         \
    static type create(TextModParser&);                 \
    constexpr static ParserRuleKind ENUM_TYPE = kind

}  // namespace rules
}  // namespace tm_parse