//
// Date       : 20/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "text_mod/text_mod_context.h"
#include "text_mod/writer_context.h"

namespace tm_parse {

str_view WriterContext::resolve(table_ref ref) const {
    if (m_Context == nullptr) {
        throw std::runtime_error{"Parse context is null!"};
    }

    return m_Context->name_table().find(ref);
}

}  // namespace tm_parse