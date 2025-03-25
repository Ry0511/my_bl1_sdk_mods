//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "object.h"

PYBIND11_MODULE(blutils, m) {
    m.def(
        "import_text",
        &blutils::import_text,
        "parent"_a,
        "prop"_a,
        "text"_a,
        "flags"_a,
        "obj_flags"_a
    );

    m.def("find_object", &blutils::find_object, "obj_path_name"_a, "bthrow"_a);
}