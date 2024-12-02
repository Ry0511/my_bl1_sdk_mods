//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "object_explorer.h"

#include "GLFW/glfw3.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace object_explorer {

// ############################################################################//
//  | GLOBAL GARBAGE |
// ############################################################################//

Context ctx{};

namespace {

int init_glfw(void);
int init_imgui(void);
int init_hooks(void);

}  // namespace

// ############################################################################//
//  | INIT |
// ############################################################################//

int initialise(void) noexcept {
    if (ctx.HasInitialised) {
        return 0;
    }

    try {
        ctx = Context{};
        init_glfw();
        init_imgui();
        init_hooks();
        ctx.HasInitialised = true;

        const char* glfw_version = glfwGetVersionString();
        const char* gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* imgui_version = ImGui::GetVersion();
        LOG(INFO, "Object Explorer Initialised");
        LOG(INFO, "  {}", gl_version);
        LOG(INFO, "  {}", glfw_version);
        LOG(INFO, "  {}", imgui_version);

        return 0;
    } catch (int code) {
        return code;
    }
}

void terminate(void) noexcept {
    if (!ctx.HasInitialised) {
        return;
    }

    std::wstring_view hook_name = L"object_explorer_actor_tick";
    std::wstring_view hook_func = L"Engine.Actor:Tick";
    hook_manager::Type hook_type = hook_manager::Type::PRE;
    hook_manager::remove_hook(hook_func, hook_type, hook_name);

    GLFWwindow* window = static_cast<GLFWwindow*>(ctx.Window);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
    ctx = Context{};
    LOG(INFO, "Object Explorer Shutdown");
}

void begin_frame(void) noexcept {
    GLFWwindow* window = static_cast<GLFWwindow*>(ctx.Window);
    if (glfwGetCurrentContext() != window) {
        glfwMakeContextCurrent(window);
    }

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
}

void end_frame(void) noexcept {
    GLFWwindow* window = static_cast<GLFWwindow*>(ctx.Window);
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

// ############################################################################//
//  | GLOBAL GARBAGE |
// ############################################################################//

namespace {

int init_glfw(void) {
    if (!glfwInit()) {
        throw -1;
    }

    glfwSetErrorCallback([](int error, const char* description) {
        LOG(ERROR, "[GLFW] ~ {}, '{}'", error, description);
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    ctx.Window = glfwCreateWindow(800, 600, "Object Explorer", nullptr, nullptr);
    if (!ctx.Window) {
        LOG(ERROR, "Failed to create Object Explorer window");
        glfwTerminate();
        throw -1;
    }

    glfwMakeContextCurrent((GLFWwindow*)ctx.Window);
    glfwSwapInterval(1);

    return 0;
}

int init_imgui(void) {
    GLFWwindow* window = static_cast<GLFWwindow*>(ctx.Window);

    if (!IMGUI_CHECKVERSION()) {
        throw -1;
    }
    void* imgui_context = ImGui::CreateContext();
    if (!imgui_context) {
        LOG(ERROR, "Failed to create imgui context");
        throw -1;
    }

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

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        LOG(ERROR, "Failed to initialise imgui glfw backend");
        throw -1;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        LOG(ERROR, "Failed to initialise imgui gl3 backend");
        throw -1;
    }

    return 0;
}

int init_hooks(void) {
    // TODO: Find a better way to integrate into the games update loop this doesn't work if the game
    //  is paused.

    std::wstring_view hook_name = L"object_explorer_actor_tick";
    std::wstring_view hook_func = L"Engine.Actor:Tick";
    hook_manager::Type hook_type = hook_manager::Type::PRE;

    if (hook_manager::has_hook(hook_func, hook_type, hook_name)) {
        return 0;
    }

    auto hook = [](const hook_manager::Details&) -> bool {
        Instant now = Clock::now();
        float delta = FloatDuration{now - ctx.LastTickTime}.count();

        if (delta < ctx.TargetFramerate) {
            return false;
        }

        Instant instant_before_frame = Clock::now();
        begin_frame();
        update();
        end_frame();
        Instant instant_after_frame = Clock::now();
        ctx.LastTickTime = now;
        ctx.LastRenderDelta = FloatDuration{instant_after_frame - instant_before_frame}.count();

        return false;
    };

    if (!hook_manager::add_hook(hook_func, hook_type, hook_name, hook)) {
        LOG(ERROR, L"Failed to add hook: {}, {}", hook_func, hook_name);
        return -1;
    }

    return 0;
}

}  // namespace
}  // namespace object_explorer