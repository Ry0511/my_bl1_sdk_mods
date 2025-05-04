//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "object_explorer.h"

#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"

#include "GLFW/glfw3.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define IMGUI_USER_CONFIG "_imconfig_.h"
#include "imgui.h"


namespace unrealsdk::unreal {
class UDelegateProperty;
class UInterfaceProperty;
class UComponentProperty;
}  // namespace unrealsdk::unreal
namespace object_explorer {
////////////////////////////////////////////////////////////////////////////////
// | CONSTANTS |
////////////////////////////////////////////////////////////////////////////////

// This always gets ticked which is what we want
static const std::wstring tick_fn_name = L"Engine.GameViewportClient:Tick";
static const std::wstring tick_fn_id = L"object_explorer_tick_fn";

////////////////////////////////////////////////////////////////////////////////
// | VARIABLES |
////////////////////////////////////////////////////////////////////////////////

struct UnrealCoreProperties {
    UClass* ArrayProperty{nullptr};
    UClass* ClassProperty{nullptr};
    UClass* StructProperty{nullptr};
    UClass* ObjectProperty{nullptr};
    UClass* NameProperty{nullptr};
    UClass* StrProperty{nullptr};
    UClass* IntProperty{nullptr};
    UClass* FloatProperty{nullptr};
    UClass* BoolProperty{nullptr};
    UClass* ByteProperty{nullptr};
    UClass* ComponentProperty{nullptr};
    UClass* InterfaceProperty{nullptr};
    UClass* MapProperty{nullptr};
    UClass* DelegateProperty{nullptr};
};

struct RuntimeInfo {
    GLFWwindow* MainWindow{nullptr};
    ImGuiContext* ImGuiContext{nullptr};
    std::unique_ptr<UnrealCoreProperties> CoreProperties{nullptr};
};

static RuntimeInfo g_RuntimeInfo{};
static std::thread g_InitThread{};
static std::atomic_bool g_HasInitialised{false};
static std::atomic_bool g_ShutdownRequested{false};

// TODO: Figure out how the fuck this synchronisation stuff is supposed to work.

using flag_t = uint8_t;
static flag_t system_init_flags = 0;

// clang-format off
constexpr flag_t FGLFW_INIT              = 1 << 0;

constexpr flag_t FIMGUI_INIT_CONTEXT     = 1 << 1;
constexpr flag_t FIMGUI_INIT_GL3         = 1 << 2;
constexpr flag_t FIMGUI_INIT_GLFW_FOR_GL = 1 << 3;

constexpr flag_t FOE_TICK_HOOKED         = 1 << 4;
// clang-format on

////////////////////////////////////////////////////////////////////////////////
// | STARTUP |
////////////////////////////////////////////////////////////////////////////////

static bool _tick_callback(const hook_manager::Details& /*details*/) {

    return false;
}

static void _glfw_error_callback(int error, const char* description) {
    LOG(ERROR, "[GLFW] Error {:#x} - '{}'", error, description);
}

static void _init_glfw3() {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error{"Failed to initialise glfw"};
    }

    glfwSetErrorCallback(&_glfw_error_callback);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    g_RuntimeInfo.MainWindow = glfwCreateWindow(800, 600, "Object Explorer", nullptr, nullptr);
    if (g_RuntimeInfo.MainWindow == nullptr) {
        glfwTerminate();
        throw std::runtime_error{"Failed to create main window"};
    }

    glfwShowWindow(g_RuntimeInfo.MainWindow);
    glfwMakeContextCurrent(g_RuntimeInfo.MainWindow);
    glfwSwapInterval(1);
    system_init_flags |= FGLFW_INIT;
}

static void _init_imgui() {
    if (!IMGUI_CHECKVERSION()) {
        throw std::runtime_error{"Failed to initialise imgui"};
    }

    g_RuntimeInfo.ImGuiContext = ImGui::CreateContext();
    if (g_RuntimeInfo.ImGuiContext == nullptr) {
        throw std::runtime_error{"Failed to create imgui context"};
    }

    system_init_flags |= FIMGUI_INIT_CONTEXT;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "object_explorer.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    io.ConfigErrorRecovery = true;
    io.ConfigErrorRecoveryEnableAssert = false;
    io.ConfigErrorRecoveryEnableDebugLog = true;
    io.ConfigErrorRecoveryEnableTooltip = true;

    if (!ImGui_ImplGlfw_InitForOpenGL(g_RuntimeInfo.MainWindow, true)) {
        throw std::runtime_error{"Failed to initialise imgui glfw backend"};
    }
    system_init_flags |= FIMGUI_INIT_GLFW_FOR_GL;

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        throw std::runtime_error{"Failed to initialise imgui gl3 backend"};
    }

    system_init_flags |= FIMGUI_INIT_GL3;
};

static void _init_object_explorer() {
    bool has_hooked =
        hook_manager::add_hook(tick_fn_name, hook_manager::Type::PRE, tick_fn_id, &_tick_callback);

    if (!has_hooked) {
        throw std::runtime_error{fmt::format("Failed to add hook to '{}'", tick_fn_name)};
    }
    system_init_flags |= FOE_TICK_HOOKED;

    g_RuntimeInfo.CoreProperties = std::make_unique<UnrealCoreProperties>();
    auto& props = *g_RuntimeInfo.CoreProperties;

    // clang-format off
    props.ArrayProperty     = find_class(L"Core.ArrayProperty"_fn);
    props.ClassProperty     = find_class(L"Core.ClassProperty"_fn);
    props.StructProperty    = find_class(L"Core.StructProperty"_fn);
    props.ObjectProperty    = find_class(L"Core.ObjectProperty"_fn);
    props.NameProperty      = find_class(L"Core.NameProperty"_fn);
    props.StrProperty       = find_class(L"Core.StrProperty"_fn);
    props.IntProperty       = find_class(L"Core.IntProperty"_fn);
    props.FloatProperty     = find_class(L"Core.FloatProperty"_fn);
    props.BoolProperty      = find_class(L"Core.BoolProperty"_fn);
    props.ByteProperty      = find_class(L"Core.ByteProperty"_fn);
    props.ComponentProperty = find_class(L"Core.ComponentProperty"_fn);
    props.InterfaceProperty = find_class(L"Core.InterfaceProperty"_fn);
    props.MapProperty       = find_class(L"Core.MapProperty"_fn);
    props.DelegateProperty  = find_class(L"Core.DelegateProperty"_fn);
    // clang-format on
}

void initialise() {
    try {
        g_InitThread = std::thread{[]() -> void {
            g_RuntimeInfo = RuntimeInfo{};
            _init_glfw3();
            _init_imgui();
            _init_object_explorer();
            g_HasInitialised = true;

            const char* glfw_version = glfwGetVersionString();
            const char* gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
            const char* imgui_version = ImGui::GetVersion();
            LOG(INFO, "Object Explorer v{} Initialised", object_explorer::version());
            LOG(INFO, " >  {}", gl_version);
            LOG(INFO, " >  {}", glfw_version);
            LOG(INFO, " >  {}", imgui_version);

            while (g_HasInitialised && !glfwWindowShouldClose(g_RuntimeInfo.MainWindow)) {

                if (g_ShutdownRequested.load()) {
                    shutdown();
                    return;
                }

                if (g_HasInitialised) {
                    begin_frame();
                    update();
                    end_frame();
                }

                if (glfwWindowShouldClose(g_RuntimeInfo.MainWindow)) {
                    glfwSetWindowShouldClose(g_RuntimeInfo.MainWindow, GLFW_TRUE);
                    g_ShutdownRequested.store(true);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        }};

        g_InitThread.detach();

    } catch (const std::runtime_error& ex) {
        // We don't deinitialise things here
        LOG(ERROR, "Failed to initialise Object Explorer: '{}'", ex.what());
        LOG(DEV_WARNING, "Init flag state {:#x}", system_init_flags);
        shutdown();
    }
}

void shutdown() {
    if (std::this_thread::get_id() != g_InitThread.get_id()) {
        g_ShutdownRequested.store(true);
        return;
    }
    LOG(INFO, "Shutting down Object Explorer...");

    hook_manager::remove_hook(tick_fn_name, hook_manager::Type::PRE, tick_fn_id);

    if (system_init_flags & FIMGUI_INIT_GL3) {
        ImGui_ImplOpenGL3_Shutdown();
    }

    if (system_init_flags & FIMGUI_INIT_GLFW_FOR_GL) {
        ImGui_ImplGlfw_Shutdown();
    }

    if (system_init_flags & FIMGUI_INIT_CONTEXT) {
        ImGui::DestroyContext(g_RuntimeInfo.ImGuiContext);
    }
    g_RuntimeInfo.ImGuiContext = nullptr;

    if ((system_init_flags & FGLFW_INIT) && g_RuntimeInfo.MainWindow != nullptr) {
        glfwDestroyWindow(g_RuntimeInfo.MainWindow);
    }
    glfwTerminate();

    g_RuntimeInfo = RuntimeInfo{};
    g_ShutdownRequested.store(false);

    // Reset flags
    system_init_flags = 0;
    g_HasInitialised = false;
}

////////////////////////////////////////////////////////////////////////////////
// | UPDATE LOOP |
////////////////////////////////////////////////////////////////////////////////

void begin_frame() {
    auto* window = g_RuntimeInfo.MainWindow;
    if (glfwGetCurrentContext() != window) {
        glfwMakeContextCurrent(window);
    }

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
}

void end_frame() {
    auto* window = g_RuntimeInfo.MainWindow;

    ImGui::Render();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);

    const ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(window);
    }
}

void update() {
    ImGui::ShowDemoWindow();

    if (ImGui::Begin("All Objects")) {
        const GObjects& objects = gobjects();
        ImGuiListClipper clipper{};
        clipper.Begin(static_cast<int>(objects.size()));

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                UObject* obj = objects.obj_at(static_cast<size_t>(i));

                if (obj == nullptr) {
                    ImGui::TextColored({1.0F, 0.0F, 0.0F, 1.0F}, "NULL");
                } else {
                    std::string text = std::format("{}", obj->get_path_name());
                    ImGui::Text(text.c_str());
                    if (ImGui::IsItemClicked()) {
                        LOG(INFO, "Clicked on {}", obj->get_path_name());
                    }
                }
            }
        }
    }

    ImGui::End();
}

////////////////////////////////////////////////////////////////////////////////
// | HELPERS |
////////////////////////////////////////////////////////////////////////////////

void draw_object_tree() {}

void draw_object_viewer() {}

bool draw_uobject_view(UObject* /*obj*/) {
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// | UPROPERTY RENDERERS |
////////////////////////////////////////////////////////////////////////////////

static void draw_arrayproperty(UObject* src, UArrayProperty* prop);
static void draw_classproperty(UObject* src, UClassProperty* prop);
static void draw_structproperty(UObject* src, UStructProperty* prop);
static void draw_objectproperty(UObject* src, UObjectProperty* prop);
static void draw_nameproperty(UObject* src, UNameProperty* prop);
static void draw_strproperty(UObject* src, UStrProperty* prop);
static void draw_intproperty(UObject* src, UIntProperty* prop);
static void draw_floatproperty(UObject* src, UFloatProperty* prop);
static void draw_boolproperty(UObject* src, UBoolProperty* prop);
static void draw_byteproperty(UObject* src, UByteProperty* prop);
static void draw_componentproperty(UObject* src, UComponentProperty* prop);
static void draw_interfaceproperty(UObject* src, UInterfaceProperty* prop);
static void draw_mapproperty(UObject* src, UProperty* prop);
static void draw_delegateproperty(UObject* src, UDelegateProperty* prop);

bool draw_property_view(UObject* obj) {
    if (obj == nullptr) {
        return false;
    }

    const UnrealCoreProperties& props = *g_RuntimeInfo.CoreProperties;
    for (UProperty* p : obj->Class->properties()) {
        const UClass* cls = p->Class;

        // clang-format off
        if (cls == props.ArrayProperty)     { draw_arrayproperty(    obj, reinterpret_cast<UArrayProperty*>(p));     continue; }
        if (cls == props.ClassProperty)     { draw_classproperty(    obj, reinterpret_cast<UClassProperty*>(p));     continue; }
        if (cls == props.StructProperty)    { draw_structproperty(   obj, reinterpret_cast<UStructProperty*>(p));    continue; }
        if (cls == props.ObjectProperty)    { draw_objectproperty(   obj, reinterpret_cast<UObjectProperty*>(p));    continue; }
        if (cls == props.NameProperty)      { draw_nameproperty(     obj, reinterpret_cast<UNameProperty*>(p));      continue; }
        if (cls == props.StrProperty)       { draw_strproperty(      obj, reinterpret_cast<UStrProperty*>(p));       continue; }
        if (cls == props.IntProperty)       { draw_intproperty(      obj, reinterpret_cast<UIntProperty*>(p));       continue; }
        if (cls == props.FloatProperty)     { draw_floatproperty(    obj, reinterpret_cast<UFloatProperty*>(p));     continue; }
        if (cls == props.BoolProperty)      { draw_boolproperty(     obj, reinterpret_cast<UBoolProperty*>(p));      continue; }
        if (cls == props.ByteProperty)      { draw_byteproperty(     obj, reinterpret_cast<UByteProperty*>(p));      continue; }
        if (cls == props.ComponentProperty) { draw_componentproperty(obj, reinterpret_cast<UComponentProperty*>(p)); continue; }
        if (cls == props.InterfaceProperty) { draw_interfaceproperty(obj, reinterpret_cast<UInterfaceProperty*>(p)); continue; }
        if (cls == props.MapProperty)       { draw_mapproperty(      obj, reinterpret_cast<UProperty*>(p));          continue; }
        if (cls == props.DelegateProperty)  { draw_delegateproperty( obj, reinterpret_cast<UDelegateProperty*>(p));  continue; }
        // clang-format on
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// | DRAW PROPERTY IMPL |
////////////////////////////////////////////////////////////////////////////////

void draw_arrayproperty(UObject* src, UArrayProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_classproperty(UObject* src, UClassProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_structproperty(UObject* src, UStructProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_objectproperty(UObject* src, UObjectProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_nameproperty(UObject* src, UNameProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_strproperty(UObject* src, UStrProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_intproperty(UObject* src, UIntProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_floatproperty(UObject* src, UFloatProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_boolproperty(UObject* src, UBoolProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_byteproperty(UObject* src, UByteProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_componentproperty(UObject* src, UComponentProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_interfaceproperty(UObject* src, UInterfaceProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_mapproperty(UObject* src, UProperty* prop) {
    (void)src;
    (void)prop;
};
void draw_delegateproperty(UObject* src, UDelegateProperty* prop) {
    (void)src;
    (void)prop;
};

}  // namespace object_explorer