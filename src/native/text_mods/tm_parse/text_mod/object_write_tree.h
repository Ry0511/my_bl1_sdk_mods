//
// Date       : 21/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "parser/rules/primary_expr_rules.h"
#include "text_mod/name_table.h"

namespace tm_parse {

class TextMod;
class TextModLoader;

// TODO: Use a small buffer here as a minor optimisation
class RefChain {
   private:
    std::vector<table_ref> m_RefChain;

   public:
    constexpr RefChain() = default;
    constexpr RefChain(table_ref ref) : m_RefChain{ref} {};
    ~RefChain() = default;

   public:
    size_t size() const noexcept { return m_RefChain.size(); }

   public:
    bool operator==(const RefChain& other) const noexcept {
        if (size() != other.size()) {
            return false;
        }

        for (size_t i = 0; i < m_RefChain.size(); ++i) {
            if (m_RefChain[i] != other.m_RefChain[i]) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const RefChain& other) const noexcept { return !this->operator==(other); }

   public:
    void push_ref(table_ref ref) noexcept { m_RefChain.push_back(ref); }
    RefChain copy_extend(table_ref ref) const noexcept;

    // clang-format off
    decltype(auto) begin() noexcept       { return m_RefChain.begin(); }
    decltype(auto) end()   noexcept       { return m_RefChain.end();   }
    decltype(auto) begin() const noexcept { return m_RefChain.begin(); }
    decltype(auto) end()   const noexcept { return m_RefChain.end();   }
    // clang-format on
};

class FlatObjectWriteList {
   public:
    using SourcedExpr = std::pair<const TextMod*, const rules::PrimitiveExprRule*>;
    // TODO: Some form of listener for when a property is overwritten would be useful for informing the user of what
    // expression is actually being used since we always overwrite, might even want to prevent the expression from being
    // overwritten.

   private:
    table_ref m_ObjRef;
    std::vector<RefChain> m_PropertyRefChains;
    std::vector<SourcedExpr> m_PropExpr;
    std::vector<FlatObjectWriteList*> m_ChildObjects;

   public:
    FlatObjectWriteList(table_ref obj_ref) : m_ObjRef(obj_ref) {};
    ~FlatObjectWriteList() = default;

   public:
    table_ref obj_ref() const noexcept { return m_ObjRef; }

   public:
    void add_set_command(TextModLoader& ctx, const TextMod& mod, const rules::SetCommandRule& rule);
    void add_obj_def(TextModLoader& ctx, const TextMod& mod, const rules::ObjectDefinitionRule& rule);

   public:
    void add_child_obj(FlatObjectWriteList& child) {
        m_ChildObjects.push_back(&child);
    }

   private:
    void _parse_expr(NameTable& table, table_ref ref, const TextMod& mod, const rules::ExpressionRule& expr);
    void _parse_expr_list(
        NameTable& table,
        const RefChain& ref_chain,
        const TextMod& mod,
        const rules::AssignmentExprListRule& expr
    );
    void _parse_leaf(const RefChain& ref_chain, const TextMod& mod, const rules::PrimitiveExprRule& expr);

   private:
    size_t _find_or_insert(const RefChain& chain) noexcept;
};

}  // namespace tm_parse