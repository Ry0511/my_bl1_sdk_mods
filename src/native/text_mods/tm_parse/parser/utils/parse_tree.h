//
// Date       : 11/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"
#include "parser/rules/primary_rules.h"

namespace tm_parse {

class ParseTree {
   public:
    ParseTree(const rules::ProgramRule& rule);
    ~ParseTree() = default;

   private:
    void visitor_func(const auto&) noexcept;
};

}  // namespace tm_parse
