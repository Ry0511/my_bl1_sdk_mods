//
// Date       : 23/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "parser/rules/primary_rules.h"
#include "text_mod/object_write_tree.h"
#include "text_mod/text_mod_loader.h"

namespace tm_parse {

using namespace rules;
using namespace utils;

////////////////////////////////////////////////////////////////////////////////
// | REF CHAIN |
////////////////////////////////////////////////////////////////////////////////

RefChain RefChain::copy_extend(table_ref ref) const noexcept {
    RefChain chain{};
    for (table_ref inner_ref : m_RefChain) {
        chain.push_ref(inner_ref);
    }
    chain.push_ref(ref);
    return chain;
}

////////////////////////////////////////////////////////////////////////////////
// | FLAT OBJECT WRITE LIST |
////////////////////////////////////////////////////////////////////////////////

void FlatObjectWriteList::add_set_command(
    TextModLoader& ctx,
    const TextMod& source,
    const rules::SetCommandRule& rule
) {
    NameTable& name_table = ctx.name_table();

    table_ref prop = name_table.register_name(rule.to_str(source.text()));
    _parse_expr(name_table, prop, source, rule.expr());
}

void FlatObjectWriteList::add_obj_def(TextModLoader& ctx, const TextMod& mod, const rules::ObjectDefinitionRule& rule) {

    NameTable& name_table = ctx.name_table();

    // NOTE: We ignore the child objects here as we only construct the property assignment tree for
    // 'this' object the caller is responsible for telling us who are parent is.

    for (const auto& assign : rule.assignments()) {
        table_ref prop = name_table.register_name(assign.property().to_str(mod.text()));
        _parse_expr(name_table, prop, mod, assign.expr());
    }

}

void FlatObjectWriteList::_parse_expr(
    NameTable& table,
    table_ref ref,
    const TextMod& mod,
    const rules::ExpressionRule& expr
) {
    RefChain ref_chain = RefChain{ref};

    if (expr.is_expr_list()) {
        TXT_MOD_ASSERT(expr.is<PrimitiveExprRule>(), "expecting expression list");
        _parse_expr_list(table, ref_chain, mod, expr.get<AssignmentExprListRule>());
    }
    else {
        TXT_MOD_ASSERT(expr.is<PrimitiveExprRule>(), "expecting primitive expr");
        _parse_leaf(ref_chain, mod, expr.get<PrimitiveExprRule>());
    }
}

void FlatObjectWriteList::_parse_expr_list(
    NameTable& table,
    const RefChain& ref_chain,
    const TextMod& mod,
    const AssignmentExprListRule& list
) {
    for (const auto& assign : list.assignments()) {
        // Skip empty assignments
        if (!assign.has_expr()) {
            continue;
        }

        const ExpressionRule& expr = assign.expr();

        table_ref prop = table.register_name(assign.property().to_str(mod.text()));
        RefChain sub_chain = ref_chain.copy_extend(prop);

        // This assignment is a list of assignments
        if (assign.expr().is_expr_list()) {
            TXT_MOD_ASSERT(expr.is<AssignmentExprListRule>(), "expecting primitive expr");
            _parse_expr_list(table, sub_chain, mod, expr.get<AssignmentExprListRule>());
        }
        // Assignment is a leaf/value producing expression
        else {
            TXT_MOD_ASSERT(expr.is<PrimitiveExprRule>(), "expecting primitive expr");
            _parse_leaf(sub_chain, mod, expr.get<PrimitiveExprRule>());
        }
    }
}

void FlatObjectWriteList::_parse_leaf(const RefChain& ref_chain, const TextMod& mod, const PrimitiveExprRule& expr) {
    size_t index = _find_or_insert(ref_chain);
    if (index == invalid_index_v) {
        m_PropExpr.emplace_back(&mod, &expr);
    } else {
        m_PropExpr[index] = std::make_pair(&mod, &expr);
    }
}

size_t FlatObjectWriteList::_find_or_insert(const RefChain& chain) noexcept {

    // Try to find the ref chain if we can
    for (size_t i = 0; i < m_PropertyRefChains.size(); ++i) {
        if (m_PropertyRefChains[i] == chain) {
            return i;
        }
    }

    m_PropertyRefChains.push_back(chain);
    return m_PropertyRefChains.size() - 1;
}

}  // namespace tm_parse