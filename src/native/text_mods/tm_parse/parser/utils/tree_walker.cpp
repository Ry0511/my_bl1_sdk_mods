//
// Date       : 11/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"
#include "parser/utils/tree_walker.h"

namespace tm_parse::utils {

void TreeWalker::walk(const rules::ProgramRule& rule, const auto& func) const noexcept {
    func(rule);
    for (const auto& inner : rule.rules()) {
        std::visit([this, &func](const auto& rule) { this->walk(rule, func); }, inner);
    }
}

void TreeWalker::walk(const rules::ObjectDefinitionRule& rule, const auto& func) const noexcept {
    func(rule);
    func(rule.clazz());
    func(rule.name());

    for (const auto& objects : rule.child_objects()) {
        this->walk(*objects, func);
    }

    for (const auto& assignment : rule.assignments()) {
        this->walk(assignment, func);
    }
}

void TreeWalker::walk(const rules::SetCommandRule& rule, const auto& func) const noexcept {
    func(rule);
    this->walk(rule.object(), func);
    this->walk(rule.property(), func);
    this->walk(rule.expr(), func);
}

void TreeWalker::walk(const rules::ObjectIdentifierRule& rule, const auto& func) const noexcept {
    func(rule);
    this->walk(rule.primary_identifier(), func);

    if (const auto& child = rule.child_identifier()) {
        this->walk(child, func);
    }
}

void TreeWalker::walk(const rules::ObjectAccessRule& rule, const auto& func) const noexcept {
    func(rule);

    if (const auto& clazz = rule.class_type()) {
        this->walk(clazz, func);
    }
    this->walk(rule.object_path(), func);
}

void TreeWalker::walk(const rules::ArrayAccessRule& rule, const auto& func) const noexcept {
    func(rule);
    this->walk(rule.index(), func);
}

void TreeWalker::walk(const rules::PropertyAccessRule& rule, const auto& func) const noexcept {
    func(rule);
    this->walk(rule.identifier(), func);

    if (const auto& array_access = rule.array_access()) {
        this->walk(array_access, func);
    }
}

void TreeWalker::walk(const rules::NameExprRule& rule, const auto& func) const noexcept {
    func(rule);
    this->walk(*rule.class_ref(), func);
    this->walk(*rule.object_ref(), func);
}

#define LEAF_IMPL(type)                                                               \
    void TreeWalker::walk(const rules::type& rule, const auto& func) const noexcept { \
        func(rule);                                                                   \
    }

LEAF_IMPL(IdentifierRule);
LEAF_IMPL(DotIdentifierRule);
//LEAF_IMPL(ObjectIdentifierRule);
//LEAF_IMPL(ObjectAccessRule);
//LEAF_IMPL(ArrayAccessRule);
//LEAF_IMPL(PropertyAccessRule);

LEAF_IMPL(NumberExprRule);
LEAF_IMPL(StrExprRule);
//LEAF_IMPL(NameExprRule);
LEAF_IMPL(KeywordRule);
LEAF_IMPL(LiteralExprRule);

LEAF_IMPL(AssignmentExprRule);
LEAF_IMPL(AssignmentExprListRule);
LEAF_IMPL(ParenExprRule);
LEAF_IMPL(ExpressionRule);

}  // namespace tm_parse::utils
