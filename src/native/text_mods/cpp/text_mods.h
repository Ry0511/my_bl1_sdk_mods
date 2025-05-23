//
// Date       : 16/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

////////////////////////////////////////////////////////////////////////////////
// | INCLUDES |
////////////////////////////////////////////////////////////////////////////////

#include "pyunrealsdk/pch.h"
#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uproperty.h"

////////////////////////////////////////////////////////////////////////////////
// | PREFACE |
////////////////////////////////////////////////////////////////////////////////

namespace unrealsdk::unreal {
class UProperty;
}

////////////////////////////////////////////////////////////////////////////////
// | MAIN INTERFACE |
////////////////////////////////////////////////////////////////////////////////

namespace text_mods {

using namespace unrealsdk;
using namespace unrealsdk::unreal;

namespace fs = std::filesystem;

void initialise();

/**
 * Imports into the given property the given string
 *
 * @param obj The object to modify
 * @param prop The property to set
 * @param text The string to import
 * @return True if the result from ImportText is not null.
 */
bool import_text(UObject* obj, UProperty* prop, std::string_view text);

/**
 * @copydoc import_text
 */
bool import_wtext(UObject* obj, UProperty* prop, std::wstring_view text);

/**
 * Performs the given set command
 * @param cmd The set command string to parse
 * @param keep_alive Should we keep the modified object alive
 * @throws std::runtime_error
 */
void invoke_set_cmd(std::wstring_view cmd, bool keep_alive = true);

/**
 * Imports the given text file and loads it into the current object scope.
 * @param txt_file The path to the file to load.
 */
void import_text_file(const fs::path& txt_file);

/**
 * Exports an object as an object definition string i.e.,
 * ```
 * Begin Object Class=Foo Name=Baz
 *   Prop=()
 * End Object
 * ```
 * @param obj The object to export
 * @return The exported object. May contain errors.
 */
std::wstring export_wtext(UObject* obj);

/**
 * Exports a property from an object
 *
 * @param obj The object that has the property
 * @param prop The property to export
 * @param index The index if the object is an Array or Dynamic Array.
 * @return The exported string
 */
std::wstring export_wtext_prop(UObject* obj, UProperty* prop, int32_t index = 0);

}  // namespace text_mods