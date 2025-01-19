//
// Date       : 28/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//
#ifndef PACKAGE_TOOLS_H
#define PACKAGE_TOOLS_H

#include "pyunrealsdk/pch.h"

#include "pyunrealsdk/pyunrealsdk.h"
#include "unrealsdk/unrealsdk.h"

namespace package_tools {

////////////////////////////////////////////////////////////////////////////////
// | GLOBAL MESS |
////////////////////////////////////////////////////////////////////////////////

using namespace unrealsdk;
using namespace unrealsdk::unreal;
using namespace pyunrealsdk;

template <class T>
using Optional = std::optional<T>;

////////////////////////////////////////////////////////////////////////////////
// | API |
////////////////////////////////////////////////////////////////////////////////

std::string version() noexcept;
void initialise();

bool is_package(const UObject* const obj) noexcept;

Optional<UObject*> find_package(const std::wstring& pkg_name, const Optional<UObject*>& pkg = {});
std::vector<UObject*> get_all_packages();

std::vector<UObject*> get_package_contents(UObject* pkg);
std::vector<UObject*> get_packages_in_package(UObject* pkg);
std::vector<UObject*> get_objects_in_package(UObject* pkg);

}  // namespace package_tools

#endif
