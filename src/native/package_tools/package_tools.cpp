//
// Date       : 27/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "package_tools.h"

#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

namespace package_tools {

namespace {
UClass* upackage_class = nullptr;
}

std::string version() noexcept {
    return "1.0.0";
}

void initialise() {
    upackage_class = find_class(L"Core.Package");
    if (upackage_class == nullptr) {
        throw std::runtime_error("Core.Package class was not found");
    }
}

bool is_package(const UObject* const obj) noexcept {
    return obj && obj->Class == upackage_class;
}

Optional<UObject*> find_package(const std::wstring& pkg_name, const Optional<UObject*>& pkg_opt) {
    if (!upackage_class) {
        throw std::runtime_error("Core.Package class was not found");
    }

    UObject* pkg = pkg_opt.value_or(nullptr);

    if (pkg != nullptr && pkg->Class != upackage_class) {
        throw std::invalid_argument("pkg must be null or a Core.Package object");
    }

    const GObjects& g = gobjects();
    auto result = g.end();

    // Search for root package
    if (!pkg) {
        result = std::find_if(g.begin(), g.end(), [&pkg_name](const UObject* obj) -> bool {
            if (!obj || obj->Class != upackage_class) {
                return false;
            }
            std::wstring path_name = obj->get_path_name();
            return path_name == pkg_name;
        });

        // Search for package inside package
    } else {
        result = std::find_if(g.begin(), g.end(), [pkg, &pkg_name](const UObject* obj) -> bool {
            if (!obj || obj->Class != upackage_class || obj->Outer != pkg) {
                return false;
            }
            std::wstring path_name = obj->get_path_name();
            return path_name == pkg_name;
        });
    }

    if (result != g.end()) {
        return std::make_optional<UObject*>(*result);
    } else {
        return std::nullopt;
    }
}

std::vector<UObject*> get_all_packages() {
    std::vector<UObject*> packages{};
    for (UObject* obj : gobjects()) {
        if (!obj || obj->Class != upackage_class || obj->Outer != nullptr) {
            continue;
        }
        packages.push_back(obj);
    }
    return packages;
}

std::vector<UObject*> get_package_contents(UObject* pkg) {
    std::vector<UObject*> contents{};

    if (!pkg || pkg->Class != upackage_class) {
        throw std::invalid_argument("pkg must be a Core.Package object");
    }

    for (UObject* obj : gobjects()) {
        if (!obj) {
            continue;
        }

        if (obj->Outer == pkg) {
            contents.push_back(obj);
        }
    }

    return contents;
}

std::vector<UObject*> get_packages_in_package(UObject* pkg) {
    if (!pkg || pkg->Class != upackage_class) {
        throw std::invalid_argument("pkg must be a Core.Package object");
    }

    std::vector<UObject*> packages{};
    for (UObject* obj : gobjects()) {
        if (!obj || obj->Class != upackage_class) {
            continue;
        }
        if (obj->Outer == pkg) {
            packages.push_back(obj);
        }
    }

    return packages;
}

std::vector<UObject*> get_objects_in_package(UObject* pkg) {
    if (!pkg || pkg->Class != upackage_class) {
        throw std::invalid_argument("pkg must be a Core.Package object");
    }

    std::vector<UObject*> objects{};
    for (UObject* obj : gobjects()) {
        if (!obj || obj->Outer != pkg || obj->Class == upackage_class) {
            continue;
        }
        objects.push_back(obj);
    }

    return objects;
}

}  // namespace package_tools