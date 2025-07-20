//
// Date       : 11/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"
#include "parser/rules/primary_rules.h"

namespace tm_parse::utils {

#define WALKER(type)            \
    template <typename Visitor> \
    void walk(const rules::type& rule, Visitor&& visitor)

// TODO Implement some form of short-circuiting

class TreeWalker {
   public:
    enum VisitType : uint8_t {
        OnEnter,
        OnExit,
    };

   public:
    constexpr TreeWalker() noexcept = default;
    ~TreeWalker() noexcept = default;

   public:
    template <class T, typename Visitor>
    void walk(const T& rule, Visitor&& visitor);

   public:
    WALKER(ProgramRule);
    WALKER(ObjectDefinitionRule);
    WALKER(SetCommandRule);

    // WALKER(IdentifierRule);
    // WALKER(DotIdentifierRule);
    WALKER(ObjectIdentifierRule);
    WALKER(ObjectAccessRule);
    WALKER(ArrayAccessRule);
    WALKER(PropertyAccessRule);

    // WALKER(NumberExprRule);
    // WALKER(StrExprRule);
    WALKER(NameExprRule);
    // WALKER(KeywordRule);
    // WALKER(LiteralExprRule);
    WALKER(PrimitiveExprRule);

    WALKER(AssignmentExprRule);
    WALKER(AssignmentExprListRule);
    WALKER(ParenExprRule);
    WALKER(ExpressionRule);
};

#undef WALKER

////////////////////////////////////////////////////////////////////////////////
// | IMPLEMENTATIONS |
////////////////////////////////////////////////////////////////////////////////

template <class T, typename Visitor>
void TreeWalker::walk(const T& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);;
    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::ProgramRule& root, Visitor&& visitor) {
    visitor(root, TreeWalker::OnEnter);

    for (const auto& inner : root.rules()) {
        std::visit(
            [this, &visitor, &root](const auto& child_rule) { this->walk(child_rule, std::forward<Visitor>(visitor)); },
            inner
        );
    }

    visitor(root, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::ObjectDefinitionRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);
    this->walk(rule.clazz(), std::forward<Visitor>(visitor));
    this->walk(rule.name(), std::forward<Visitor>(visitor));

    for (const auto& obj : rule.child_objects()) {
        this->walk(*obj, std::forward<Visitor>(visitor));
    }

    for (const auto& assignment : rule.assignments()) {
        this->walk(assignment, std::forward<Visitor>(visitor));
    }

    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::SetCommandRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);
    this->walk(rule.object(), std::forward<Visitor>(visitor));
    this->walk(rule.property(), std::forward<Visitor>(visitor));
    this->walk(rule.expr(), std::forward<Visitor>(visitor));
    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::AssignmentExprRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);
    this->walk(rule.property(), std::forward<Visitor>(visitor));
    if (rule.has_expr()) {
        this->walk(rule.expr(), std::forward<Visitor>(visitor));
    }
    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::AssignmentExprListRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);

    for (const auto& assign : rule.assignments()) {
        this->walk(assign, std::forward<Visitor>(visitor));
    }

    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::ParenExprRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);

    // Collapses (((1))) to ParenExpr( NumberExpr(1) )
    if (const auto* inner = rule.inner_most()) {
        this->walk(*inner, std::forward<Visitor>(visitor));
    }

    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::ExpressionRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);

    std::visit(
        [this, &visitor](const auto& inner) -> void {
            using T = std::decay_t<decltype(inner)>;
            if constexpr (!std::is_same_v<T, std::monostate>) {
                this->walk(inner, std::forward<Visitor>(visitor));
            }
        },
        rule.inner()
    );

    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::NameExprRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);

    if (rule.class_ref() != nullptr) {
        this->walk(*rule.class_ref(), std::forward<Visitor>(visitor));
    }
    this->walk(*rule.object_ref(), std::forward<Visitor>(visitor));

    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::ObjectAccessRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);

    if (const auto& cls = rule.class_type()) {
        this->walk(cls, std::forward<Visitor>(visitor));
    }

    this->walk(rule.object_path(), std::forward<Visitor>(visitor));

    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::PropertyAccessRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);

    this->walk(rule.identifier(), std::forward<Visitor>(visitor));

    if (const auto& array = rule.array_access()) {
        this->walk(array, std::forward<Visitor>(visitor));
    }

    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::ObjectIdentifierRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);

    this->walk(rule.primary_identifier(), std::forward<Visitor>(visitor));

    if (const auto& child = rule.child_identifier()) {
        this->walk(child, std::forward<Visitor>(visitor));
    }

    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::ArrayAccessRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);
    this->walk(rule.index(), std::forward<Visitor>(visitor));
    visitor(rule, TreeWalker::OnExit);
}

template <typename Visitor>
void TreeWalker::walk(const rules::PrimitiveExprRule& rule, Visitor&& visitor) {
    visitor(rule, TreeWalker::OnEnter);

    std::visit(
        [this, &visitor](const auto& inner) -> void {
            using T = std::decay_t<decltype(inner)>;
            if constexpr (!std::is_same_v<T, std::monostate>) {
                this->walk(inner, std::forward<Visitor>(visitor));
            }
        },
        rule.inner()
    );

    visitor(rule, TreeWalker::OnExit);
}

}  // namespace tm_parse::utils
