//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//
#ifndef BL1_SDK_MODS_OBJECT_EXPLORER_H
#define BL1_SDK_MODS_OBJECT_EXPLORER_H

#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/pyunrealsdk.h"
#include "unrealsdk/unrealsdk.h"

#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"

#define IMGUI_USER_CONFIG "_imconfig_.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace object_explorer {

using namespace unrealsdk;
using namespace unrealsdk::unreal;
using namespace unrealsdk::hook_manager;

using Clock = std::chrono::steady_clock;
using Instant = std::chrono::steady_clock::time_point;
using FloatDuration = std::chrono::duration<float>;

struct Context {
    void* Window;                           // Window Handle
    int Width, Height;                      // Window Size
    bool HasInitialised;                    // GLFW + ImGui Init statee
    Instant LastTickTime;                   // Last time we rendered/updated the ui
    float LastRenderDelta;                  // Time it took to render the last ui frame
    float TargetFramerate = 1.0F / 120.0F;  // The target framerate delta
};

extern Context ctx;

// ############################################################################//
//  | UTILS |
// ############################################################################//

std::string wstr_to_str(const std::wstring& wstr) noexcept;
std::wstring str_to_wstr(const std::string& str) noexcept;

void create_view_object_tree(UObject*) noexcept;

// ############################################################################//
//  | INIT |
// ############################################################################//

int initialise(void) noexcept;
void terminate(void) noexcept;

void begin_frame(void) noexcept;
void update(void) noexcept;
void end_frame(void) noexcept;

}  // namespace object_explorer

#endif
