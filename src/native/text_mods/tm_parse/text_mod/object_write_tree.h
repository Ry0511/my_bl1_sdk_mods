//
// Date       : 21/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "text_mod/name_table.h"

namespace tm_parse {

class TextMod;
class TextModLoader;

namespace rules {
class ExpressionRule;
class AssignmentExprListRule;
class PrimitiveExprRule;
class SetCommandRule;
class ObjectDefinitionRule;
}

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

    // Helper for printing
    str full_name(const NameTable& nt) const {
        strstream ss{};
        for (size_t i = 0; i < m_RefChain.size(); ++i) {
            ss << nt.find(m_RefChain[i]);
            if (i < m_RefChain.size() - 1) {
                ss << TXT('.');
            }
        }
        return ss.str();
    }

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

    // TODO: make this stl compliant?
    class ParallelArrayIterator {
       private:
        FlatObjectWriteList* m_List;
        size_t m_Index;

       public:
        ParallelArrayIterator(FlatObjectWriteList* list, size_t index) noexcept : m_List(list), m_Index(index) {}
        ~ParallelArrayIterator() = default;

       public:
        bool operator==(const ParallelArrayIterator& other) const noexcept {
            TXT_MOD_ASSERT(m_List == other.m_List);
            return m_Index == other.m_Index;
        }

        bool operator!=(const ParallelArrayIterator& other) const noexcept { return !this->operator==(other); }

       public:
        void operator++() noexcept { ++m_Index; }
        void operator--() noexcept { --m_Index; }

       public:
        const RefChain& property() const noexcept {
            _check_self();
            return m_List->m_PropertyRefChains[m_Index];
        }

        const SourcedExpr& operator*() const noexcept {
            _check_self();
            return m_List->m_PropExpr[m_Index];
        }

        const SourcedExpr* operator->() const noexcept {
            _check_self();
            return &m_List->m_PropExpr[m_Index];
        }

       private:
        void _check_self() const noexcept {
            TXT_MOD_ASSERT(
                m_List != nullptr && m_Index < m_List->m_PropertyRefChains.size(),
                "list {:p} or index {} is invalid",
                static_cast<void*>(m_List),
                m_Index
            );
        }
    };

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

    const std::vector<RefChain>& property_ref_chains() const noexcept { return m_PropertyRefChains; }
    const std::vector<SourcedExpr>& sourced_expressions() const noexcept { return m_PropExpr; }
    const std::vector<FlatObjectWriteList*>& child_objects() const noexcept { return m_ChildObjects; }

    ParallelArrayIterator begin() noexcept { return ParallelArrayIterator{this, 0}; }
    ParallelArrayIterator end() noexcept { return ParallelArrayIterator{this, m_PropertyRefChains.size()}; }

   public:
    void add_set_command(TextModLoader& ctx, const TextMod& mod, const rules::SetCommandRule& rule);
    void add_obj_def(TextModLoader& ctx, const TextMod& mod, const rules::ObjectDefinitionRule& rule);

   public:
    void add_child_obj(FlatObjectWriteList& child) { m_ChildObjects.push_back(&child); }

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

   public:
};

}  // namespace tm_parse