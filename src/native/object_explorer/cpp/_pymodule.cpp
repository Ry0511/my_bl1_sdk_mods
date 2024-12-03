//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "object_explorer.h"

PYBIND11_MODULE(object_explorer, m) {

    m.def("initialise", []() {
        LOG(INFO, "Initialising object explorer...");
        int res = object_explorer::initialise();
        if (res < 0) {
            LOG(ERROR, "Fatal error occured during initialisation process...");
        } else if (res > 0) {
            LOG(WARNING, "Non fatal error occured during initialisation process..");
        }
        return res;
    });

    m.def("terminate", []() {
        LOG(INFO, "Terminating object explorer...");
        object_explorer::terminate();
    });
}