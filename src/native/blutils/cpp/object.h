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

#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uproperty.h"

namespace blutils {

using namespace unrealsdk;
using namespace unreal;

/**
 * Modifies an objects property via a string.
 * @param obj The object to set the property on
 * @param prop The property to set
 * @param text The value string for the property to be set by
 * @param import_flags The text import flags (advanced)
 * @param obj_flags The flags to set on 'obj'
 * @return True if the object was modified, false otherwise.
 */
bool import_text(
    UObject* obj,
    UProperty* prop,
    const std::wstring& text,
    uint32_t import_flags,
    uint64_t obj_flags
);

/**
 * Imports/Creates an object from a full `Begin Object ... End Object` block
 * @param base_pkg The base package name to create into.
 * @param text The import object string.
 * @return A vector containing all objects created or modified.
 */
std::vector<UObject*> import_object(const std::wstring& base_pkg, const std::string& text);

/**
 * Finds an object by its full path name.
 * @param obj_path_name The full path name of the object to find
 * @param bthrow If we should throw if don't find an object
 * @return The object found, or an empty optional
 */
std::optional<UObject*> find_object(const std::wstring& obj_path_name, bool bthrow);

}  // namespace blutils

#endif
