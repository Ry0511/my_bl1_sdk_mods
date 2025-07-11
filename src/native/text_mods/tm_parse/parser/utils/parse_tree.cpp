//
// Date       : 11/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"
#include "parser/utils/parse_tree.h"
#include "parser/utils/tree_walker.h"

namespace tm_parse {

using namespace rules;

class ParseTree {
   private:
    static void visitor_func(const auto& rule) noexcept {
        utils::TreeWalker<std::decay_t<decltype(rule)>> walker{};
        walker.walk(rule);
    }

   public:
    ParseTree(const ProgramRule& root) {
        root.visit(&visitor_func);
    }
};

}  // namespace tm_parse