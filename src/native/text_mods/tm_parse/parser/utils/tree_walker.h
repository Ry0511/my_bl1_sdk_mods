//
// Date       : 11/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"
#include "parser/rules/primary_rules.h"

namespace tm_parse::utils {

#define WALKER(type)                  \
    template <visitor_struct Visitor> \
    void walk(const rules::type& rule, Visitor&& visitor)

// TODO Implement some form of short-circuiting

enum TreeWalkerVisitType : uint8_t {
    OnEnter,
    OnExit,
};

template <class T>
concept visitor_struct = std::is_class_v<T>;

class TreeWalker {
   private:
    template <class Function>
    struct VisitorFunctionWrapper {
        Function&& func;
        void on_enter(auto&& rule) { func(rule, OnEnter); }
        void on_exit(auto&& rule) { func(rule, OnExit); }
    };

   public:
    constexpr TreeWalker() noexcept = default;
    ~TreeWalker() noexcept = default;

   public:
    template <class T, visitor_struct Visitor>
    void walk(const T& rule, Visitor&& visitor);

    template <class T, class Visitor, typename = std::enable_if_t<!visitor_struct<Visitor>>>
    void walk(const T& rule, Visitor&& visitor) {
        using Wrapper = VisitorFunctionWrapper<Visitor>;
        this->walk(rule, Wrapper{std::forward<Visitor>(visitor)});
    }

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

template <class T, visitor_struct Visitor>
void TreeWalker::walk(const T& rule, Visitor&& visitor) {
    visitor.on_enter(rule);
    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::ProgramRule& root, Visitor&& visitor) {
    visitor.on_enter(root);

    for (const auto& inner : root.rules()) {
        std::visit(
            [this, &visitor, &root](const auto& child_rule) { this->walk(child_rule, std::forward<Visitor>(visitor)); },
            inner
        );
    }

    visitor.on_exit(root);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::ObjectDefinitionRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);
    this->walk(rule.clazz(), std::forward<Visitor>(visitor));
    this->walk(rule.name(), std::forward<Visitor>(visitor));

    for (const auto& obj : rule.child_objects()) {
        this->walk(*obj, std::forward<Visitor>(visitor));
    }

    for (const auto& assignment : rule.assignments()) {
        this->walk(assignment, std::forward<Visitor>(visitor));
    }

    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::SetCommandRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);
    this->walk(rule.object(), std::forward<Visitor>(visitor));
    this->walk(rule.property(), std::forward<Visitor>(visitor));
    this->walk(rule.expr(), std::forward<Visitor>(visitor));
    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::AssignmentExprRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);
    this->walk(rule.property(), std::forward<Visitor>(visitor));
    // Forget if this is nullable, but expr() derefs regardless...
    this->walk(rule.expr(), std::forward<Visitor>(visitor));
    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::AssignmentExprListRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);

    for (const auto& assign : rule.assignments()) {
        this->walk(assign, std::forward<Visitor>(visitor));
    }

    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::ParenExprRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);

    // Collapses (((1))) to ParenExpr( NumberExpr(1) )
    if (const auto* inner = rule.inner_most()) {
        this->walk(*inner, std::forward<Visitor>(visitor));
    }

    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::ExpressionRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);

    std::visit(
        [this, &visitor](const auto& inner) -> void {
            using T = std::decay_t<decltype(inner)>;
            if constexpr (!std::is_same_v<T, std::monostate>) {
                this->walk(inner, std::forward<Visitor>(visitor));
            }
        },
        rule.inner()
    );

    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::NameExprRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);

    if (rule.class_ref() != nullptr) {
        this->walk(*rule.class_ref(), std::forward<Visitor>(visitor));
    }
    this->walk(*rule.object_ref(), std::forward<Visitor>(visitor));

    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::ObjectAccessRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);

    if (const auto& cls = rule.class_type()) {
        this->walk(cls, std::forward<Visitor>(visitor));
    }

    this->walk(rule.object_path(), std::forward<Visitor>(visitor));

    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::PropertyAccessRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);

    this->walk(rule.identifier(), std::forward<Visitor>(visitor));

    if (const auto& array = rule.array_access()) {
        this->walk(array, std::forward<Visitor>(visitor));
    }

    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::ObjectIdentifierRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);

    this->walk(rule.primary_identifier(), std::forward<Visitor>(visitor));

    if (const auto& child = rule.child_identifier()) {
        this->walk(child, std::forward<Visitor>(visitor));
    }

    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::ArrayAccessRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);
    this->walk(rule.index(), std::forward<Visitor>(visitor));
    visitor.on_exit(rule);
}

template <visitor_struct Visitor>
void TreeWalker::walk(const rules::PrimitiveExprRule& rule, Visitor&& visitor) {
    visitor.on_enter(rule);

    std::visit(
        [this, &visitor](const auto& inner) -> void {
            using T = std::decay_t<decltype(inner)>;
            if constexpr (!std::is_same_v<T, std::monostate>) {
                this->walk(inner, std::forward<Visitor>(visitor));
            }
        },
        rule.inner()
    );

    visitor.on_exit(rule);
}

}  // namespace tm_parse::utils
