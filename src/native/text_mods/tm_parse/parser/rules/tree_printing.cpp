//
// Date       : 07/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"
#include "parser/rules/parser_rules.h"
#include "parser/rules/primary_expr_rules.h"
#include "parser/rules/primary_rules.h"
#include "parser/rules/primitive_expr_rules.h"

namespace tm_parse::rules {

constexpr int indent_size = 2;

auto append_rule(strstream& ss, const auto& rule, int indent) {
    using T = std::decay_t<decltype(rule)>;
    ss << str(indent, txt::lit::space) << str{rule_name(T::ENUM_TYPE)} << txt::lit::lf;
}

#define LEAF_NODE_IMPL(clazz)                                   \
    void clazz::append_tree(strstream& ss, int& indent) const { \
        append_rule(ss, *this, indent);                         \
    }

// Base Rules
LEAF_NODE_IMPL(IdentifierRule);
LEAF_NODE_IMPL(DotIdentifierRule);

// Primitive Expressions
LEAF_NODE_IMPL(NumberExprRule);
LEAF_NODE_IMPL(StrExprRule);
LEAF_NODE_IMPL(NameExprRule);
LEAF_NODE_IMPL(KeywordRule);
LEAF_NODE_IMPL(LiteralExprRule);

void ObjectIdentifierRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;

    primary_identifier().append_tree(ss, indent);

    if (child_identifier()) {
        child_identifier().append_tree(ss, indent);
    }

    indent -= indent_size;
}

void ObjectAccessRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;

    if (class_type()) {
        class_type().append_tree(ss, indent);
    }

    object_path().append_tree(ss, indent);

    indent -= indent_size;
}

void ArrayAccessRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;
    index().append_tree(ss, indent);
    indent -= indent_size;
}

void PropertyAccessRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;

    identifier().append_tree(ss, indent);

    if (array_access()) {
        array_access().append_tree(ss, indent);
    }

    indent -= indent_size;
}

void PrimitiveExprRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;
    std::visit(
        [&ss, &indent](const auto& rule) {
            using T = std::decay_t<decltype(rule)>;
            if constexpr (!std::is_same_v<T, std::monostate>) {
                rule.append_tree(ss, indent);
            }
        },
        m_InnerRule
    );
    indent -= indent_size;
}

// Primary Expressions
void AssignmentExprRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;
    m_Property.append_tree(ss, indent);

    if (has_expr()) {
        m_Expr->append_tree(ss, indent);
    }
    indent -= indent_size;
}

void AssignmentExprListRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;

    for (const auto& assignment : m_Assignments) {
        assignment.append_tree(ss, indent);
    }

    indent -= indent_size;
}

void ParenExprRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;

    if (m_Expr != nullptr) {
        m_Expr->append_tree(ss, indent);
    }
    indent -= indent_size;
}

void ExpressionRule::append_tree(strstream& ss, int& indent) const {
    // Only print the child
    std::visit(
        [&ss, &indent](const auto& rule) {
            using T = std::decay_t<decltype(rule)>;
            if constexpr (!std::is_same_v<T, std::monostate>) {
                rule.append_tree(ss, indent);
            };
        },
        m_InnerType
    );
}

// Primary Rules
void SetCommandRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);

    indent += indent_size;
    this->object().append_tree(ss, indent);
    this->property().append_tree(ss, indent);
    this->expr().append_tree(ss, indent);

    indent -= indent_size;
}

void ObjectDefinitionRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;

    this->clazz().append_tree(ss, indent);
    this->name().append_tree(ss, indent);
    for (const auto& child : this->child_objects()) {
        child->append_tree(ss, indent);
    }

    for (const auto& assignment : this->assignments()) {
        assignment.append_tree(ss, indent);
    }

    indent -= indent_size;
}

void ProgramRule::append_tree(strstream& ss, int& indent) const {
    append_rule(ss, *this, indent);
    indent += indent_size;

    for (const auto& rule : m_Rules) {
        std::visit([&ss, &indent](const auto& rule) -> void { rule.append_tree(ss, indent); }, rule);
    }

    indent -= indent_size;
}

}  // namespace tm_parse::rules