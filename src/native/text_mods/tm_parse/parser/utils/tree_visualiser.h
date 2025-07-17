//
// Date       : 07/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"
#include "parser/rules/primary_rules.h"

namespace tm_parse {

void print_tree(const rules::ProgramRule& rule, strstream& ss);
void print_tree(const rules::SetCommandRule& rule, strstream& ss);
void print_tree(const rules::ObjectDefinitionRule& rule, strstream& ss);

}