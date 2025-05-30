//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//
#ifndef BL1_SDK_MODS_OBJECT_EXPLORER_H
#define BL1_SDK_MODS_OBJECT_EXPLORER_H

#include "pyunrealsdk/pch.h"
#include "unrealsdk/pch.h"
#include "pyunrealsdk/pyunrealsdk.h"
#include "unrealsdk/unrealsdk.h"

#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

namespace object_explorer {

using namespace unrealsdk;
using namespace unrealsdk::unreal;
using namespace unrealsdk::hook_manager;

using Clock = std::chrono::steady_clock;
using Instant = std::chrono::steady_clock::time_point;
using FloatDuration = std::chrono::duration<float>;

constexpr int OBJECT_EXPLORER_VER_MAJOR = 0;
constexpr int OBJECT_EXPLORER_VER_MINOR = 0;

inline std::string version() noexcept {
    return fmt::format("{}.{}", OBJECT_EXPLORER_VER_MAJOR, OBJECT_EXPLORER_VER_MINOR);
}

void initialise(void);
void shutdown(void);

void begin_frame(void);
void end_frame(void);
void update(void);

////////////////////////////////////////////////////////////////////////////////
// | APPLICATION VIEWS |
////////////////////////////////////////////////////////////////////////////////

void draw_debug_view(void);
void draw_all_objects_view(void);
void draw_outer_tree_view(void);
void draw_edit_object_view(void);

////////////////////////////////////////////////////////////////////////////////
// | SINGLE OBJECT RENDERS |
////////////////////////////////////////////////////////////////////////////////

int draw_obj_as_package_tree(UObject* pkg);
int draw_editable_property(UObject* src, UProperty* prop);

}  // namespace object_explorer

#endif
