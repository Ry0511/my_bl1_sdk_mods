//
// Date       : 07/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "parser/utils/tree_visualiser.h"
#include "common/text_mod_utils.h"

namespace tm_parse {

using namespace txt;

void print_tree_internal(const auto& rule) {
    strstream ss{};
    int indent = 0;
    rule.append_tree(ss, indent);

    TXT_LOG("Program Tree");
    for (str line; std::getline(ss, line);) {
        TXT_LOG("{}", line);
    }
}

void print_tree(const rules::ProgramRule& rule) {
    print_tree_internal(rule);
}

void print_tree(const rules::SetCommandRule& rule) {
    print_tree_internal(rule);
}

void print_tree(const rules::ObjectDefinitionRule& rule) {
    print_tree_internal(rule);
}

}  // namespace tm_parse