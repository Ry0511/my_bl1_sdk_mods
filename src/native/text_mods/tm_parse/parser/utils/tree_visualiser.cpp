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

    auto fn = [&indent, &ss](const auto& rule, TreeWalker::VisitType visit_type) -> void {

        if (visit_type == TreeWalker::OnEnter) {
            indent += 2;
            ss << str(indent, ' ') << str{rule.enum_name()} << txt::lit::lf;

        } else if (visit_type == TreeWalker::OnExit) {
            indent -= 2;
        }
    };

    walker.walk(rule, fn);
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