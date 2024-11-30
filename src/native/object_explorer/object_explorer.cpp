//
// Date       : 29/11/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pyunrealsdk/pch.h"
#include <unrealsdk/unreal/classes/properties/uboolproperty.h>
#include <unrealsdk/unreal/classes/properties/uobjectproperty.h>

#include "unrealsdk/unrealsdk.h"

#include "unrealsdk/hook_manager.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"

#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "windows.h"

// TODO: Figure out what to do with this.

namespace {

using namespace unrealsdk;
using namespace unrealsdk::unreal;

std::string wstr_to_str(const std::wstring& wstr) {
    int buffer_size = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        static_cast<int>(wstr.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (buffer_size == 0) {
        return std::string{};
    }

    std::string utf8_str(buffer_size, '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        static_cast<int>(wstr.size()),
        utf8_str.data(),
        buffer_size,
        nullptr,
        nullptr
    );

    return utf8_str;
}

using Clock = std::chrono::steady_clock;

struct {
    int width, height;
    GLFWwindow* window = nullptr;
    bool is_initialised = false;
    bool is_running = false;
    float target_fps = 1.0F / 120.0F;
    Clock::time_point last_tick_tp{};
} ctx;

void start_frame() {
    if (!ctx.is_running || !ctx.is_initialised) {
        return;
    }

    if (glfwGetCurrentContext() != ctx.window) {
        glfwMakeContextCurrent(ctx.window);
    }

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
}

void end_frame() {
    if (!ctx.is_running || !ctx.is_initialised) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        if (ctx.window) {
            glfwDestroyWindow(ctx.window);
        }
        glfwTerminate();
        ctx = {};
        return;
    }

    ImGui::Render();
    glfwGetFramebufferSize(ctx.window, &ctx.width, &ctx.height);
    glViewport(0, 0, ctx.width, ctx.height);
    glClearColor(0.2F, 0.2F, 0.2F, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(ctx.window);

    const ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(ctx.window);
    }
}

void init() {
    if (glfwInit() != GLFW_TRUE) {
        LOG(ERROR, "Failed to initialise glfw!");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    ctx.window = glfwCreateWindow(800, 600, "Object Explorer", nullptr, nullptr);

    if (!ctx.window) {
        LOG(ERROR, "Failed to create window!");
        return;
    }

    glfwMakeContextCurrent(ctx.window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "object_explorer.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(ctx.window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ctx.is_running = true;
    ctx.is_initialised = true;
}

void push_tree_for_uobject(UObject* obj) {
    std::string path_name = wstr_to_str(obj->get_path_name());
    std::string cls = (obj->Class) ? wstr_to_str(obj->Class->get_path_name()) : std::string{"NULL"};
    std::string id = "0x" + std::to_string((uintptr_t)obj);

    struct TypeParser {
        UClass* Class;
        std::function<void(UObject*, UProperty*)> Parser = &do_nothing;
        static void do_nothing(UObject*, UProperty*) { /* Does something */ }
    };

    static std::array<TypeParser, 8> primitive_types{
        TypeParser{
            find_class(L"Core.IntProperty"),
            [](UObject* obj, UProperty* prop) {
                ImGui::Text(
                    "Value: %i",
                    get_property<UIntProperty>((UIntProperty*)prop, 0, (uintptr_t)obj)
                );
            }
        },
        TypeParser{
            find_class(L"Core.FloatProperty"),
            [](UObject* obj, UProperty* prop) {
                ImGui::Text(
                    "Value: %f",
                    get_property<UFloatProperty>((UFloatProperty*)prop, 0, (uintptr_t)obj)
                );
            }
        },
        TypeParser{
            find_class(L"Core.BoolProperty"),
            [](UObject* obj, UProperty* prop) {
                bool val = get_property<UBoolProperty>((UBoolProperty*)prop, 0, (uintptr_t)obj);
                ImGui::Text("Value: %s", val ? "True" : "False");
            }
        },
        TypeParser{
            find_class(L"Core.NameProperty"),
            [](UObject* obj, UProperty* prop) {
                FName name = get_property<UNameProperty>((UNameProperty*)prop, 0, (uintptr_t)obj);
                ImGui::Text("Value: %s", name.operator std::string().c_str());
            }
        },
        TypeParser{
            find_class(L"Core.StrProperty"),
            [](UObject* obj, UProperty* prop) {
                std::wstring str =
                    get_property<UStrProperty>((UStrProperty*)prop, 0, (uintptr_t)obj);
                ImGui::Text("Value: %s", wstr_to_str(str).c_str());
            }
        },
        TypeParser{
            find_class(L"Core.ByteProperty"),
            [](UObject* obj, UProperty* prop) {
                uint8_t val = get_property<UByteProperty>((UByteProperty*)prop, 0, (uintptr_t)obj);
                ImGui::Text("Value: %i", val);
            }
        },
        TypeParser{
            find_class(L"Core.StructProperty"),
            [](UObject* obj, UProperty* prop) {
                WrappedStruct val =
                    get_property<UStructProperty>((UStructProperty*)prop, 0, (uintptr_t)obj);

                if (!val.type) {
                    ImGui::Text("NULL");
                    return;
                }

                if (ImGui::TreeNode(obj->Name.operator std::string().c_str())) {
                    for (auto p : val.type->properties()) {
                        if (ImGui::TreeNode(p->Name.operator std::string().c_str())) {
                            bool found = false;
                            for (const auto& parser : primitive_types) {
                                if (p->Class == parser.Class) {
                                    parser.Parser(obj, p);
                                    found = true;
                                    break;
                                }
                            }

                            if (!found) {
                                ImGui::Text("Value: Unknown");
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }
        },
        TypeParser{
            find_class(L"Core.ObjectProperty"),
            [](UObject* obj_, UProperty* prop) {
                UObject* val =
                    get_property<UObjectProperty>((UObjectProperty*)prop, 0, (uintptr_t)obj_);

                if (!val) {
                    ImGui::Text("NULL");
                    return;
                }

                if (ImGui::TreeNode(val->Name.operator std::string().c_str())) {
                    for (auto p : val->Class->properties()) {
                        if (ImGui::TreeNode(p->Name.operator std::string().c_str())) {
                            if (val->Outer) {
                                ImGui::Text(
                                    "Outer: %s",
                                    wstr_to_str(val->Outer->get_path_name()).c_str()
                                );
                            } else {
                                ImGui::Text("Outer: NULL");
                            }
                            ImGui::Text("Path : %s", wstr_to_str(p->get_path_name()).c_str());
                            ImGui::Text(
                                "Type : %s",
                                (p->Class ? p->Class->Name : std::string{"NULL"}).c_str()
                            );
                            ImGui::Text("Size : %i", p->ElementSize);

                            bool found = false;
                            for (const auto& parser : primitive_types) {
                                if (p->Class == parser.Class) {
                                    parser.Parser(val, p);
                                    found = true;
                                    break;
                                }
                            }

                            if (!found) {
                                ImGui::Text("Value: Unknown");
                            }

                            ImGui::TreePop();
                        }
                    }

                    ImGui::TreePop();
                }
            }
        }
    };

    if (ImGui::TreeNode(id.c_str(), "%s", path_name.c_str())) {
        for (auto p : obj->Class->properties()) {
            if (ImGui::TreeNode(p->Name.operator std::string().c_str())) {
                if (obj->Outer) {
                    ImGui::Text("Outer: %s", wstr_to_str(obj->Outer->get_path_name()).c_str());
                } else {
                    ImGui::Text("Outer: NULL");
                }
                ImGui::Text("Path : %s", wstr_to_str(p->get_path_name()).c_str());
                ImGui::Text("Type : %s", (p->Class ? p->Class->Name : std::string{"NULL"}).c_str());
                ImGui::Text("Size : %i", p->ElementSize);

                bool found = false;
                for (const auto& parser : primitive_types) {
                    if (p->Class == parser.Class) {
                        parser.Parser(obj, p);
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    ImGui::Text("Value: Unknown");
                }

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void update() {
    ImGui::ShowDemoWindow();

    static int selected_index = -1;
    const GObjects& objects = gobjects();

    if (ImGui::Begin("All Objects")) {
        ImGuiListClipper clipper{};
        clipper.Begin(objects.size());

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                UObject* obj = objects.obj_at(i);
                if (!obj) {
                    ImGui::Selectable("NULL", false, ImGuiSelectableFlags_Disabled);
                    continue;
                }
                std::string path_name = wstr_to_str(obj->get_path_name());
                if (ImGui::Selectable(path_name.c_str())) {
                    selected_index = i;
                }
            }
        }
        clipper.End();
    }
    ImGui::End();

    if (ImGui::Begin("Selected Object")) {
        size_t index = static_cast<size_t>(selected_index);
        if (index >= objects.size()) {
            ImGui::Text("Invalid Object");
            goto selected_object_exit;
        }

        UObject* obj = objects.obj_at(selected_index);
        if (!obj) {
            ImGui::Text("Invalid Object Selected!");
            goto selected_object_exit;
        }

        push_tree_for_uobject(obj);
    }
selected_object_exit:
    ImGui::End();
}

}  // namespace

PYBIND11_MODULE(object_explorer, m) {
    m.def("get_version", []() { return std::string{"0.0"}; });

    m.def("start", []() {
        if (ctx.is_initialised) {
            return;
        }

        if (hook_manager::has_hook(
                L"Engine.Actor:Tick",
                hook_manager::Type::PRE,
                L"object_explorer_tick"
            )) {
            return;
        }

        LOG(INFO, "[ObjectExplorer] Adding Hooks");
        ctx.last_tick_tp = Clock::now();

        hook_manager::add_hook(
            L"Engine.Actor:Tick",
            hook_manager::Type::PRE,
            L"object_explorer_tick",
            [](const hook_manager::Details&) {
                if (!ctx.is_initialised) {
                    init();
                }

                if (!ctx.is_running) {
                    return false;
                }

                auto now = Clock::now();
                float delta = std::chrono::duration<float>(now - ctx.last_tick_tp).count();
                if (delta < ctx.target_fps) {
                    return false;
                }

                start_frame();
                update();
                end_frame();
                ctx.last_tick_tp = now;
                return false;
            }
        );
    });

    m.def("stop", []() { ctx.is_running = false; });
}