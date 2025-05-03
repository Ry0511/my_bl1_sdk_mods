//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "object_explorer.h"

PYBIND11_MODULE(object_explorer, m) {

    m.def("initialise", []() {
        LOG(INFO, "Initialising object explorer...");
        object_explorer::initialise();
    });

    m.def("terminate", []() {
        LOG(INFO, "Terminating object explorer...");
        object_explorer::shutdown();
    });
}