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
#include "unrealsdk/unreal/classes/properties/ucomponentproperty.h"
#include "unrealsdk/unreal/classes/properties/udelegateproperty.h"
#include "unrealsdk/unreal/classes/properties/uinterfaceproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/wrappers/weak_pointer.h"

#include "GLFW/glfw3.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define IMGUI_USER_CONFIG "_imconfig_.h"
#include "imgui.h"

#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#define OELOG(level, msg, ...) LOG(level, "[OBJECT_EXPLORER] ~ {}", fmt::format(msg, __VA_ARGS__))

namespace object_explorer {

// TODO: This needs to be ripped apart and put into several source files.

////////////////////////////////////////////////////////////////////////////////
// | CONSTANTS |
////////////////////////////////////////////////////////////////////////////////

// This always gets ticked which is what we want
static const std::wstring tick_fn_name = L"Engine.GameViewportClient:Tick";
static const std::wstring tick_fn_id = L"object_explorer_tick_fn";

////////////////////////////////////////////////////////////////////////////////
// | VARIABLES |
////////////////////////////////////////////////////////////////////////////////

// Worth checking to see if these have a consistent memory address I would assume they are class
//  static variables. If so we can just reinterpret the addresses.
struct UnrealCoreProperties {
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
    UClass* PackageClass{nullptr};
    UClass* ClassClass{nullptr};
};

struct RuntimeInfo {
    // Startup
    std::thread ApplicationThread{};
    std::thread::id ApplicationThreadID{};
    GLFWwindow* MainWindow{nullptr};
    ImGuiContext* ImGuiContext{nullptr};
    std::atomic_bool HasInitialised{false};
    std::atomic_bool ShutdownRequested{false};

    // Loop Signal
    std::atomic_flag TickPingPongFlag{};

    // Runtime
    std::unique_ptr<UnrealCoreProperties> CoreProperties{nullptr};

   public:
    inline void reset() noexcept {
        ApplicationThread = std::thread{};
        MainWindow = nullptr;
        ImGuiContext = nullptr;
        HasInitialised.store(false);
        ShutdownRequested.store(false);

        TickPingPongFlag.clear();

        CoreProperties = nullptr;
    }

    inline bool is_app_thread() const noexcept {
        return std::this_thread::get_id() == ApplicationThreadID;
    }
};

// Internal runtime state
static RuntimeInfo g_RuntimeInfo{};

// The mutex used when initialising and deinitialising
static std::mutex g_SystemMutex{};
static std::condition_variable g_SystemCondVar{};

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
    auto& info = g_RuntimeInfo;

    if (!info.HasInitialised.load()) {
        OELOG(INFO, "Tick callback executed whilst not initialised?");
        return false;
    }

    // Avoid waiting on shutdown
    if (info.ShutdownRequested.load()) {
        info.TickPingPongFlag.test_and_set();
        info.TickPingPongFlag.notify_one();
        return false;
    }

    // Tell the _app_loop to perform one frame
    info.TickPingPongFlag.test_and_set();
    info.TickPingPongFlag.notify_one();

    // Wait until the _app_loop has completed its frame
    info.TickPingPongFlag.wait(true);

    return false;
}

static void _app_loop() noexcept {
    while (true) {
        auto& info = g_RuntimeInfo;

        info.TickPingPongFlag.wait(false);

        // Handle any shutdown requests
        if (glfwWindowShouldClose(info.MainWindow) == GLFW_TRUE) {
            g_RuntimeInfo.ShutdownRequested.store(true, std::memory_order_release);
        }

        // Invoke any shutdown requests
        if (info.ShutdownRequested.load()) {
            shutdown();
            info.TickPingPongFlag.clear();
            info.TickPingPongFlag.notify_one();
            return;
        }

        begin_frame();
        update();
        end_frame();

        info.TickPingPongFlag.clear();
        info.TickPingPongFlag.notify_one();
    }
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
    g_RuntimeInfo.CoreProperties = std::make_unique<UnrealCoreProperties>();
    UnrealCoreProperties& props = *g_RuntimeInfo.CoreProperties;

    // TODO: Might be able to replace these via a call to FName{"CLASS_NAME"}
    //  i.e.,
    //    static const auto package_cls = FName{"Package"};
    //

    // clang-format off
    props.ClassProperty     = find_class(L"Core.ClassProperty");
    props.StructProperty    = find_class(L"Core.StructProperty");
    props.ObjectProperty    = find_class(L"Core.ObjectProperty");
    props.NameProperty      = find_class(L"Core.NameProperty");
    props.StrProperty       = find_class(L"Core.StrProperty");
    props.IntProperty       = find_class(L"Core.IntProperty");
    props.FloatProperty     = find_class(L"Core.FloatProperty");
    props.BoolProperty      = find_class(L"Core.BoolProperty");
    props.ByteProperty      = find_class(L"Core.ByteProperty");
    props.ComponentProperty = find_class(L"Core.ComponentProperty");
    props.InterfaceProperty = find_class(L"Core.InterfaceProperty");
    props.MapProperty       = find_class(L"Core.MapProperty");
    props.DelegateProperty  = find_class(L"Core.DelegateProperty");
    props.PackageClass      = find_class(L"Core.Package");
    props.ClassClass        = find_class(L"Core.Class");
    // clang-format on

    bool has_hooked =
        hook_manager::add_hook(tick_fn_name, hook_manager::Type::PRE, tick_fn_id, &_tick_callback);

    if (!has_hooked) {
        throw std::runtime_error{fmt::format("Failed to add hook to '{}'", tick_fn_name)};
    }
    system_init_flags |= FOE_TICK_HOOKED;
}

void initialise() {
    try {
        g_RuntimeInfo.ApplicationThread = std::thread{[]() -> void {
            std::unique_lock<std::mutex> guard{g_SystemMutex};

            if (g_RuntimeInfo.HasInitialised.load()) {
                return;
            }

            // On the off-chance we acquired the guard before the shutdown function does we will
            //  wait until it shuts the system down before we initialise the system again.
            g_SystemCondVar.wait(guard, []() { return !g_RuntimeInfo.ShutdownRequested.load(); });

            g_RuntimeInfo.reset();
            g_RuntimeInfo.ApplicationThreadID = std::this_thread::get_id();
            _init_glfw3();
            _init_imgui();
            _init_object_explorer();
            g_RuntimeInfo.HasInitialised.store(true);

            const char* glfw_version = glfwGetVersionString();
            const char* gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
            const char* imgui_version = ImGui::GetVersion();
            LOG(INFO, "Object Explorer v{} Initialised", object_explorer::version());
            LOG(INFO, " >  OpenGL {}", gl_version);
            LOG(INFO, " >  GLFW   {}", glfw_version);
            LOG(INFO, " >  ImGui  {}", imgui_version);

            guard.unlock();
            g_SystemCondVar.notify_all();
            _app_loop();
        }};

        g_RuntimeInfo.ApplicationThread.detach();

    } catch (const std::runtime_error& ex) {
        // We don't deinitialise things here
        LOG(ERROR, "Failed to initialise Object Explorer: '{}'", ex.what());
        LOG(DEV_WARNING, "Init flag state {:#x}", system_init_flags);
        shutdown();
    }
}

void shutdown() {
    if (!g_RuntimeInfo.HasInitialised.load()) {
        OELOG(INFO, "Shutdown requested but not initialised");
        return;
    }

    if (!g_RuntimeInfo.is_app_thread()) {
        LOG(INFO, "Shutdown requested from non-app thread");
        g_RuntimeInfo.ShutdownRequested.store(true);
        return;
    }

    {
        LOG(INFO, "Shutting down");
        std::unique_lock guard{g_SystemMutex};

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

        // Reset globals
        g_RuntimeInfo.reset();
        system_init_flags = 0;
    }

    g_SystemCondVar.notify_all();
}

////////////////////////////////////////////////////////////////////////////////
// | UPDATE LOOP |
////////////////////////////////////////////////////////////////////////////////

void begin_frame() {
    auto* window = g_RuntimeInfo.MainWindow;
    if (glfwGetCurrentContext() != window) {
        glfwMakeContextCurrent(window);
    }

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

    glfwPollEvents();
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
    auto draw_view = [](auto&& fn) -> void {
        ImGuiErrorRecoveryState state{};
        try {
            ImGui::ErrorRecoveryStoreState(&state);
            fn();
        } catch (...) {
            ImGui::ErrorRecoveryTryToRecoverState(&state);
        }
    };

    draw_view(&draw_debug_view);
    draw_view(&draw_all_objects_view);
    draw_view(&draw_outer_tree_view);
}

////////////////////////////////////////////////////////////////////////////////
// | DEBUG VIEW |
////////////////////////////////////////////////////////////////////////////////

void draw_debug_view() {
    if (!ImGui::Begin("Debug")) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Runtime Info Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto tree_text = [](const std::string&& title, const std::string&& value) -> void {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(title.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(value.c_str());
        };

        constexpr auto table_flags = ImGuiTableFlags_Resizable;
        if (ImGui::BeginTable("##RuntimeInfo_KV_View", 2, table_flags)) {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            const auto& info = g_RuntimeInfo;

            // clang-format off
            tree_text("ApplicationThread",   fmt::format("{:p}", (void*)&info.ApplicationThread  ));
            tree_text("ApplicationThreadID", fmt::format("{:p}", (void*)&info.ApplicationThreadID));
            tree_text("MainWindow",          fmt::format("{:p}", (void*)info.MainWindow          ));
            tree_text("ImGuiContext",        fmt::format("{:p}", (void*)info.ImGuiContext        ));
            tree_text("HasInitialised",      fmt::format("{}",   info.HasInitialised.load()      ));
            tree_text("ShutdownRequested",   fmt::format("{}",   info.ShutdownRequested.load()   ));
            tree_text("TickPingPongFlag",    fmt::format("{}",   info.TickPingPongFlag.test()    ));
            // clang-format on

            if (ImGui::TreeNode("CoreProperties")) {
                const auto& props = *g_RuntimeInfo.CoreProperties;
                // clang-format off
                tree_text("ClassProperty",     fmt::format("{:p}", (void*)props.ClassProperty    ));
                tree_text("StructProperty",    fmt::format("{:p}", (void*)props.StructProperty   ));
                tree_text("ObjectProperty",    fmt::format("{:p}", (void*)props.ObjectProperty   ));
                tree_text("NameProperty",      fmt::format("{:p}", (void*)props.NameProperty     ));
                tree_text("StrProperty",       fmt::format("{:p}", (void*)props.StrProperty      ));
                tree_text("IntProperty",       fmt::format("{:p}", (void*)props.IntProperty      ));
                tree_text("FloatProperty",     fmt::format("{:p}", (void*)props.FloatProperty    ));
                tree_text("BoolProperty",      fmt::format("{:p}", (void*)props.BoolProperty     ));
                tree_text("ByteProperty",      fmt::format("{:p}", (void*)props.ByteProperty     ));
                tree_text("ComponentProperty", fmt::format("{:p}", (void*)props.ComponentProperty));
                tree_text("InterfaceProperty", fmt::format("{:p}", (void*)props.InterfaceProperty));
                tree_text("MapProperty",       fmt::format("{:p}", (void*)props.MapProperty      ));
                tree_text("DelegateProperty",  fmt::format("{:p}", (void*)props.DelegateProperty ));
                tree_text("PackageClass",      fmt::format("{:p}", (void*)props.PackageClass     ));
                // clang-format on
                ImGui::TreePop();
            }

            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("Metrics", ImGuiTreeNodeFlags_DefaultOpen)) {}

    ImGui::End();
}

////////////////////////////////////////////////////////////////////////////////
// | ALL OBJECTS VIEW |
////////////////////////////////////////////////////////////////////////////////

void draw_all_objects_view() {
    // TODO: Clarify the design

    if (ImGui::Begin("All Objects")) {
        constexpr auto text_input_flags =
            ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_EnterReturnsTrue;

        static std::string query{};
        static std::vector<WeakPointer> filtered_objects{};
        static bool only_package_children = false;

        const GObjects& all_objects = gobjects();

        bool check_box_changed = ImGui::Checkbox("Only Package Children", &only_package_children);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        bool query_str_changed = ImGui::InputText("Query", &query, text_input_flags);

        if (check_box_changed || query_str_changed) {
            filtered_objects.clear();
            std::wstring wide_query = utils::widen(query);

            const auto should_collect = [&wide_query](const UObject* const obj) {
                return obj != nullptr && obj->Outer != nullptr
                       && (!only_package_children
                           || (only_package_children
                               && obj->Outer->Class == g_RuntimeInfo.CoreProperties->PackageClass))
                       && obj->get_path_name().find(wide_query) != std::wstring::npos;
            };

            // Collect all objects matching the query
            for (size_t i = 0; i < all_objects.size(); ++i) {
                UObject* obj = all_objects.obj_at(i);
                if (should_collect(obj)) {
                    filtered_objects.emplace_back(obj);
                }
            }

            filtered_objects.shrink_to_fit();
        }

        ImGuiListClipper clipper{};
        clipper.Begin(static_cast<int>(filtered_objects.size()));

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                const WeakPointer& obj = filtered_objects[i];

                if (!obj) {
                    ImGui::TextColored({1.0F, 0.0F, 0.0F, 1.0F}, "NULL");
                } else {
                    std::string text = std::format("{}", (*obj)->get_path_name());
                    static WeakPointer last_selected{};

                    if (ImGui::Selectable(text.c_str(), last_selected && *last_selected == *obj)) {
                        last_selected = obj;
                        OELOG(INFO, "Object Selected: {}", (*obj)->get_path_name());
                    }
                }
            }
        }
    }

    ImGui::End();
}

////////////////////////////////////////////////////////////////////////////////
// | ALL OUTER TREE |
////////////////////////////////////////////////////////////////////////////////

static UClass* g_OuterClassFilter = nullptr;
static UClass* g_ClassFilter = nullptr;

struct ObjectTreeNode {
    WeakPointer Root;
    std::optional<std::vector<ObjectTreeNode>> Children;

    void load_children() {
        if (Children.has_value() || !Root) {
            return;
        }

        Children = std::make_optional<std::vector<ObjectTreeNode>>();
        Children->reserve(32);

        // Collect children... wait
        const GObjects& all_objects = gobjects();
        for (const UObject* obj : all_objects) {
            if (obj != nullptr && obj->Outer == *Root
                && (!g_ClassFilter || obj->Class == g_ClassFilter)) {
                Children->emplace_back(obj, std::nullopt);
            }
        }

        std::sort(Children->begin(), Children->end(), &ObjectTreeNode::sort_func);

        Children->shrink_to_fit();
    }

    void reset() {
        if (Children.has_value()) {
            Children.reset();
        }
    }

    static bool sort_func(const ObjectTreeNode& lhs, const ObjectTreeNode& rhs) {
        // clang-format off
        if (!lhs.Root && !rhs.Root) return false;
        if (!lhs.Root) return false;
        if (!rhs.Root) return true;
        // clang-format on

        UClass* left = (*lhs.Root)->Class;
        UClass* right = (*rhs.Root)->Class;
        return left->Name.operator std::wstring() < right->Name.operator std::wstring();
    }
};

std::optional<std::vector<ObjectTreeNode>> g_OuterTreeNodes{};

// NOLINTNEXTLINE(*-no-recursion)
void impl_draw_outer_tree_view(ObjectTreeNode* parent, ObjectTreeNode& node, int depth) {
    // Root is null, reset the node
    if (!node.Root) {
        ImGui::TextColored({1.0F, 0.0F, 0.0F, 1.0F}, "NULL");
        node.reset();
        return;
    }

    constexpr auto flags = ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
    const UObject* const root = *node.Root;
    std::string name = fmt::format("{}##{:p}{}", (std::string)root->Name, (void*)parent, depth);

    if (ImGui::TreeNodeEx(name.c_str(), flags)) {
        node.load_children();  // Lazily load
        for (ObjectTreeNode& child : *node.Children) {
            impl_draw_outer_tree_view(&node, child, depth + 1);
        }
        ImGui::TreePop();

    } else {
        node.reset();
    }
}

void draw_outer_tree_view() {
    static std::string query_str{};

    if (!ImGui::Begin("Object Tree")) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Refresh")) {
        g_OuterTreeNodes = std::nullopt;
    }

    constexpr auto combo_flags = ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_WidthFitPreview;
    const char* filter_options[] = {"All", "Packages", "Classes"};
    static int filter_idx = 0;

    ImGui::SameLine();
    if (ImGui::BeginCombo("Filter", filter_options[filter_idx], combo_flags)) {
        for (int i = 0; i < IM_ARRAYSIZE(filter_options); ++i) {
            if (ImGui::Selectable(filter_options[i], filter_idx == i)) {
                if (filter_idx != i) {
                    g_OuterTreeNodes = std::nullopt;
                }

                const auto& info = *g_RuntimeInfo.CoreProperties;
                filter_idx = i;

                switch (filter_idx) {
                    case 0:  // All
                        g_OuterClassFilter = nullptr;
                        g_ClassFilter = nullptr;
                        break;
                    case 1:  // Packages
                        g_OuterClassFilter = info.PackageClass;
                        g_ClassFilter = nullptr;
                        break;
                    case 2:  // Classes
                        g_OuterClassFilter = nullptr;
                        g_ClassFilter = g_RuntimeInfo.CoreProperties->ClassClass;
                        break;
                    default:
                        break;
                }
            }
        }

        ImGui::EndCombo();
    }

    // TODO: Rethink this
    if (ImGui::BeginChild("Tree View")) {
        if (!g_OuterTreeNodes.has_value()) {
            g_OuterTreeNodes = std::make_optional<std::vector<ObjectTreeNode>>();
            g_OuterTreeNodes->reserve(128);

            const GObjects& all_objects = gobjects();
            for (const UObject* obj : all_objects) {
                if (obj == nullptr || obj->Outer != nullptr) {
                    continue;
                }
                g_OuterTreeNodes->emplace_back(obj, std::nullopt);
            }

            std::sort(
                g_OuterTreeNodes->begin(),
                g_OuterTreeNodes->end(),
                &ObjectTreeNode::sort_func
            );
            g_OuterTreeNodes->shrink_to_fit();
        }

        // Draw root nodes
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0F);
        for (auto it = g_OuterTreeNodes->begin(); it != g_OuterTreeNodes->end();) {
            ObjectTreeNode& node = *it;

            // Render as null text
            if (!node.Root) {
                ++it;
                impl_draw_outer_tree_view(nullptr, node, 0);
                continue;
            }

            // Render under grouped classes
            const UClass* cls = (*node.Root)->Class;
            std::string name = cls->Name;

            if (ImGui::TreeNodeEx(name.c_str())) {
                do {
                    impl_draw_outer_tree_view(nullptr, node, 0);
                    if (++it == g_OuterTreeNodes->end()) {
                        break;
                    }
                    node = *it;
                } while (!node.Root || (*node.Root)->Class == cls);
                ImGui::TreePop();

                // Skip
            } else {
                do {
                    if (++it == g_OuterTreeNodes->end()) {
                        break;
                    }
                    node = *it;
                } while (!node.Root || (*node.Root)->Class == cls);
            }
        }
        ImGui::PopStyleVar();
    }

    ImGui::EndChild();
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

}  // namespace object_explorerobject_explorer_EXPORTS