//
// Date       : 07/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/utils/tree_visualiser.h"
#include "parser/utils/tree_walker.h"

namespace tm_parse {

using namespace txt;

void print_tree_internal(const auto& rule, strstream& ss) {
    using namespace utils;
    TreeWalker walker{};
    int indent = -2;
    walker.walk(rule, [&indent, &ss](const auto& rule, TreeWalker::VisitType vt) -> void {

        if (vt == TreeWalker::OnExit) {
            indent -= 2;
        } else if (vt == TreeWalker::OnEnter) {
            indent += 2;
            ss << str(indent, ' ') << str{rule.enum_name()} << txt::lit::lf;
        }
    });
}

void print_tree(const rules::ProgramRule& rule, strstream& ss) {
    print_tree_internal(rule, ss);
}

void print_tree(const rules::SetCommandRule& rule, strstream& ss) {
    print_tree_internal(rule, ss);
}

void print_tree(const rules::ObjectDefinitionRule& rule, strstream& ss) {
    print_tree_internal(rule, ss);
}

}  // namespace tm_parse