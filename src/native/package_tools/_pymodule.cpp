//
// Date       : 27/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pyunrealsdk/pch.h"
#include "package_tools.h"

PYBIND11_MODULE(package_tools, m) {
    using namespace package_tools;

    initialise();
    m.def("version", &version);

    m.def("find_package", &find_package, "pkg_name"_a, py::arg("pkg_opt") = std::nullopt);
    m.def("is_package", &is_package, "obj"_a);

    m.def("get_all_packages", &get_all_packages);
    m.def("get_package_contents", &get_package_contents, "upk_root"_a);
    m.def("get_packages_in_package", &get_packages_in_package, "pkg"_a);
    m.def("get_objects_in_package", &get_objects_in_package, "pkg"_a);

    m.def("get_launch_args", []() -> std::optional<std::string> {
        char* arg_line = GetCommandLineA();
        if (arg_line) {
            return std::make_optional(arg_line);
        } else {
            return std::nullopt;
        }
    });
}