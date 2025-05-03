//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "object_explorer.h"

#include "GLFW/glfw3.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

namespace object_explorer {

////////////////////////////////////////////////////////////////////////////////
// | CONSTANTS |
////////////////////////////////////////////////////////////////////////////////

static const std::wstring update_func_name = L"Engine.Actor:Tick";
static const std::wstring update_func_id = L"object_explorer_update_func";

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
};

struct RuntimeInfo {
    std::unique_ptr<UnrealCoreProperties> CoreProperties;
};

static RuntimeInfo g_RuntimeInfo{};
static GLFWwindow* g_Window = nullptr;
static ImGuiContext* g_ImGuiContext = nullptr;

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

static bool _tick_callback(const hook_manager::Details& /* details */) {
    // TODO: Limit tick rate to reasonable limit
    static auto last_tick = Clock::now() + std::chrono::seconds{1};
    const float target_frame_rate = 1.0F / 120.0F;
    using FloatDuration = std::chrono::duration<float>;

    const auto now = Clock::now();

    if (FloatDuration(now - last_tick).count() < target_frame_rate) {
        return false;
    }

    last_tick = now;
    begin_frame();
    update();
    end_frame();

    return false;
}

static void _glfw_error_callback(int error, const char* description) {
    LOG(ERROR, "[GLFW] Error {:#x} - '{}'", error, description);
}

static void _init_glfw3() {
    if (!glfwInit()) {
        throw std::runtime_error{"Failed to initialise glfw"};
    }

    glfwSetErrorCallback(&_glfw_error_callback);

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_DEPTH_BITS, 8);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    g_Window = glfwCreateWindow(800, 600, "Object Explorer", nullptr, nullptr);
    if (!g_Window) {
        glfwTerminate();
        throw std::runtime_error{"Failed to create main window"};
    }

    glfwMakeContextCurrent(g_Window);
    glfwSwapInterval(1);
    system_init_flags |= FGLFW_INIT;
}

static void _init_imgui() {
    if (!IMGUI_CHECKVERSION()) {
        throw std::runtime_error{"Failed to initialise imgui"};
    }

    g_ImGuiContext = ImGui::CreateContext();
    if (g_ImGuiContext == nullptr) {
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
    io.ConfigErrorRecoveryEnableAssert = true;
    io.ConfigErrorRecoveryEnableDebugLog = true;
    io.ConfigErrorRecoveryEnableTooltip = true;

    if (!ImGui_ImplGlfw_InitForOpenGL(g_Window, true)) {
        throw std::runtime_error{"Failed to initialise imgui glfw backend"};
    }
    system_init_flags |= FIMGUI_INIT_GLFW_FOR_GL;

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        throw std::runtime_error{"Failed to initialise imgui gl3 backend"};
    }

    system_init_flags |= FIMGUI_INIT_GL3;
};

static void _init_object_explorer() {
    bool has_hooked = hook_manager::add_hook(
        update_func_name,
        hook_manager::Type::PRE,
        update_func_id,
        &_tick_callback
    );

    if (!has_hooked) {
        throw std::runtime_error{"Failed to add hook"};
    }
    system_init_flags |= FOE_TICK_HOOKED;

    g_RuntimeInfo = RuntimeInfo{};
    g_RuntimeInfo.CoreProperties = std::make_unique<UnrealCoreProperties>();

    auto& props = *g_RuntimeInfo.CoreProperties;

    // clang-format off
    // TODO: MapProperty, DelegateProperty
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
    // clang-format on
}

void initialise() {
    try {
        _init_glfw3();
        _init_imgui();
        _init_object_explorer();

        const char* glfw_version = glfwGetVersionString();
        const char* gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* imgui_version = ImGui::GetVersion();
        LOG(INFO, "Object Explorer v{} Initialised", object_explorer::version());
        LOG(INFO, " >  {}", gl_version);
        LOG(INFO, " >  {}", glfw_version);
        LOG(INFO, " >  {}", imgui_version);

    } catch (const std::runtime_error& ex) {
        // We don't deinitialise things here
        LOG(ERROR, "Failed to initialise Object Explorer: '{}'", ex.what());
        LOG(DEV_WARNING, "Init flag state {:#x}", system_init_flags);
        shutdown();
    }
}

void shutdown() {
    LOG(INFO, "Shutting down Object Explorer...");

    hook_manager::remove_hook(update_func_name, hook_manager::Type::PRE, update_func_id);

    if (system_init_flags & FIMGUI_INIT_GL3) {
        ImGui_ImplOpenGL3_Shutdown();
    }

    if (system_init_flags & FIMGUI_INIT_GLFW_FOR_GL) {
        ImGui_ImplGlfw_Shutdown();
    }

    if (system_init_flags & FIMGUI_INIT_CONTEXT) {
        ImGui::DestroyContext(g_ImGuiContext);
    }
    g_ImGuiContext = nullptr;

    if ((system_init_flags & FGLFW_INIT) && g_Window != nullptr) {
        glfwDestroyWindow(g_Window);
    }
    glfwTerminate();

    g_RuntimeInfo = RuntimeInfo{};

    // Reset flags
    system_init_flags = 0;
}

////////////////////////////////////////////////////////////////////////////////
// | UPDATE LOOP |
////////////////////////////////////////////////////////////////////////////////

void begin_frame() {}
void end_frame() {}
void update() {}

////////////////////////////////////////////////////////////////////////////////
// | HELPERS |
////////////////////////////////////////////////////////////////////////////////

void draw_object_tree() {}

void draw_object_viewer() {}

bool draw_uobject_view(UObject* /*obj*/) {}

////////////////////////////////////////////////////////////////////////////////
// | UPROPERTY RENDERERS |
////////////////////////////////////////////////////////////////////////////////

static void _draw_ustr_prop();

bool draw_property_view(UObject* /* obj */, UProperty* /* prop */) {}

}  // namespace object_explorer