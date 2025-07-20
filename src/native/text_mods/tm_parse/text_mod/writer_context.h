//
// Date       : 20/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"
#include "text_mod/name_table.h"

namespace tm_parse {

namespace rules {
class ExpressionRule;
}

class WriterContext {
   private:
    class TextModContext* m_Context;

   protected:
    str_view resolve(table_ref ref) const;

   public:
    WriterContext() = default;
    virtual ~WriterContext() = default;

   public:
    virtual void write_object(table_ref obj, table_ref property, const rules::ExpressionRule* expr) = 0;
};

}  // namespace tm_parse
