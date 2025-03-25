//
// Date       : 23/03/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//
#ifndef OBJECT_H
#define OBJECT_H

#include "pyunrealsdk/pch.h"

#include "pyunrealsdk/pyunrealsdk.h"
#include "unrealsdk/unrealsdk.h"

#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"

namespace blutils {

using namespace unrealsdk;
using namespace unreal;

bool import_text(
    UObject* obj,
    UProperty* prop,
    const std::wstring& text,
    uint32_t import_flags,
    uint64_t obj_flags
);

std::optional<UObject*> find_object(const std::wstring& obj_path_name, bool bthrow);

}  // namespace blutils

#endif
