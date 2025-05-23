//
// Date       : 16/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pyunrealsdk/pch.h"

#include "text_mods.h"

PYBIND11_MODULE(text_mods, m) {
    text_mods::initialise();
    m.def("export_text", &text_mods::export_wtext, "obj"_a);
    m.def("export_text_prop", &text_mods::export_wtext_prop, "obj"_a, "prop"_a, "index"_a = 0);
    m.def("import_text", &text_mods::import_text, "obj"_a, "prop"_a, "text"_a);
};