//
// Date       : 09/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include "tm_parse/common/text_mod_common.h"
#include "tm_parse/lexer/text_mod_lexer.h"
#include "tm_parse/parser/text_mod_parser.h"
#include "tm_parse/parser/utils/tree_walker.h"

#define LOG_THAT_SHIT(...) __VA_OPT__(std::cout << std::format(__VA_ARGS__) << std::endl)

////////////////////////////////////////////////////////////////////////////////
// | GLOBALS |
////////////////////////////////////////////////////////////////////////////////

namespace {

using namespace tm_parse;
using namespace tm_parse::utils;
using namespace tm_parse::rules_enum;

ImGuiContext* g_Context = nullptr;
ImFont* g_CascadiaCodeFont = nullptr;
GLFWwindow* g_Window = nullptr;
bool g_Running = true;

std::unique_ptr<fs::path> g_CurrentFile{};
ProgramRule g_ProgramRule{};
std::unique_ptr<std::string> g_ProgramTextTree{};

fs::path g_TextEditorFile = fs::current_path() / L"text_editor_content.txt";
std::string g_TextEditorBuffer{};
str g_CurrentTextBuffer{};

TokenTextView g_CurrentTextSelection{};
bool g_SelectionChanged = false;

// In the tree view avoid pushing for redundant nodes i.e., ExpressionRule and PrimitiveExprRule
bool g_IgnoreProxyRules = false;
bool g_KeepProgramRule = true;

std::string g_LatestErrorMessage{};

#define UID(base) std::format("{}##{}", base, __COUNTER__).c_str()

////////////////////////////////////////////////////////////////////////////////
// | OTHER STUFF CAN IGNORE |
////////////////////////////////////////////////////////////////////////////////

static void set_selection(size_t start, size_t end) {
    // Don't know how to get this to be exposed without the compiler bitching
    struct STB_TexteditState_Clone {
        int cursor;
        int select_start;
        int select_end;
        unsigned char insert_mode;
        int row_count_per_page;
        unsigned char cursor_at_end_of_line;
        unsigned char initialized;
        unsigned char has_preferred_x;
        unsigned char single_line;
        unsigned char padding1, padding2, padding3;
        float preferred_x;
    };

    auto* s = ImGui::GetInputTextState(ImGui::GetItemID());

    if (s == nullptr) {
        return;
    }

    s->CursorFollow = true;
    auto* ptr = reinterpret_cast<STB_TexteditState_Clone*>(s->Stb);
    ptr->select_start = start;
    ptr->select_end = end;
    ptr->cursor = end;
    ptr->has_preferred_x = 0;
}

////////////////////////////////////////////////////////////////////////////////
// | FUNCTIONS |
////////////////////////////////////////////////////////////////////////////////

static void glfw_error_callback(int error, const char* description) {
    LOG_THAT_SHIT("[GLFW_ERROR] 0x{:x} '{}'", error, description);
}

static int initialise();
static void update();

////////////////////////////////////////////////////////////////////////////////
// | UPDATE |
////////////////////////////////////////////////////////////////////////////////

static void _text_editor();
static void _tree_view();
static void _tree_text_view();
static void _error_view();

void update() {
    _tree_view();
    _tree_text_view();
    _text_editor();
    _error_view();
}

static void _text_editor() {
    if (!ImGui::Begin(UID("Text Editor"))) {
        ImGui::End();
        return;
    }

    if (g_SelectionChanged && g_CurrentTextSelection.is_valid()) {
        ImGui::SetKeyboardFocusHere(1);
    }

    ImGui::InputTextMultiline(
        UID("Text Editor"),
        &g_TextEditorBuffer,
        ImGui::GetContentRegionAvail(),
        ImGuiInputTextFlags_AutoSelectAll
    );

    if (g_SelectionChanged && g_CurrentTextSelection.is_valid()) {
        set_selection(g_CurrentTextSelection.Start, g_CurrentTextSelection.end());
        g_SelectionChanged = false;
    }

    if (ImGui::IsWindowFocused() && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        try {
            g_CurrentTextBuffer = to_str<str>(g_TextEditorBuffer);
            TextModLexer lexer{g_CurrentTextBuffer};
            TextModParser parser{&lexer};
            g_ProgramRule = ProgramRule::create(parser);
            g_ProgramTextTree = nullptr;

        } catch (const ParsingError& err) {
            g_LatestErrorMessage = err.build_error_message(g_CurrentTextBuffer);
        } catch (const std::exception& err) {
            g_LatestErrorMessage = err.what();
        }

        try {
            std::ofstream file{g_TextEditorFile};
            std::string content = g_TextEditorBuffer;
            file << content;
        } catch (const std::exception& err) {
            LOG_THAT_SHIT("Failed to save file: {}", err.what());
        }
    }

    ImGui::End();
}

static void _error_view() {
    if (!ImGui::Begin(UID("Error View"))) {
        ImGui::End();
        return;
    }

    if (g_LatestErrorMessage.empty()) {
        g_LatestErrorMessage = "The error if any had no message!";
    }

    ImGui::InputTextMultiline(
        UID("Error Text View"),
        &g_LatestErrorMessage,
        ImGui::GetContentRegionAvail(),
        ImGuiInputTextFlags_ReadOnly
    );

    ImGui::End();
}

////////////////////////////////////////////////////////////////////////////////
// | TREE VIEW |
////////////////////////////////////////////////////////////////////////////////

struct Visitor {
    struct PushInfo {
        const void* Identifier;
        bool IsTreeOpen;
    };

    std::deque<PushInfo> stack{};

    void on_enter(const auto& rule);
    void on_exit(const auto& rule);
};

static void _tree_view() {
    if (!ImGui::Begin(UID("Tree View"))) {
        ImGui::End();
        return;
    }

    if (!g_ProgramRule.operator bool()) {
        ImGui::Text("Program rule is invalid");
    } else {
        TreeWalker walker{};
        Visitor visitor{};

        ImGui::Checkbox("Hide Proxy Rules", &g_IgnoreProxyRules);
        ImGui::SameLine();
        ImGui::Checkbox("Keep Program Rule", &g_KeepProgramRule);

        // Not required but just for validating the template actually works
        auto fn = [&visitor](const auto& rule, TreeWalker::VisitType type) -> void {
            if (type == TreeWalker::OnEnter) {
                visitor.on_enter(rule);
            } else if (type == TreeWalker::OnExit) {
                visitor.on_exit(rule);
            }
        };

        if (ImGui::BeginChild("##tree_view", ImGui::GetContentRegionAvail())) {
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 4.0F);
            walker.walk(g_ProgramRule, fn);
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

static void _tree_text_view() {
    if (!ImGui::Begin(UID("Tree Text View"))) {
        ImGui::End();
        return;
    }

    if (g_ProgramRule.operator bool()) {
        if (g_ProgramTextTree == nullptr) {
            g_ProgramTextTree = std::make_unique<std::string>();

            strstream content{};
            int indent = 0;
            g_ProgramRule.append_tree(content, indent);
            g_ProgramTextTree->assign(content.str());
        }

        ImGui::InputTextMultiline(
            UID("Text Editor"),
            g_ProgramTextTree->data(),
            g_ProgramTextTree->size(),
            ImGui::GetContentRegionAvail(),
            ImGuiInputTextFlags_ReadOnly
        );

    } else {
        ImGui::Text("Program rule is invalid, Ctrl+S in the Text Editor creates a tree");
    }

    ImGui::End();
}

////////////////////////////////////////////////////////////////////////////////
// | INITIALISE / SHUTDOWN |
////////////////////////////////////////////////////////////////////////////////

static int initialise() {
    glfwSetErrorCallback(&glfw_error_callback);

    if (glfwInit() != GLFW_TRUE) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    g_Window = glfwCreateWindow(1280, 720, "treevis", NULL, NULL);
    if (!g_Window) {
        return 1;
    }
    glfwMakeContextCurrent(g_Window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    g_Context = ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(g_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Change the font ( if found )
    fs::path fonts_dir = fs::path("C:/Windows/Fonts");
    for (const auto& entry : fs::directory_iterator(fonts_dir)) {
        const fs::path& file = entry.path();
        std::string filename = entry.path().filename().generic_string();

        if (file.extension() != ".ttf") {
            continue;
        }

        if (filename.find("CascadiaMono") != std::string::npos) {
            g_CascadiaCodeFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(file.generic_string().c_str(), 15.0F);
            LOG_THAT_SHIT("Loaded Cascadia Mono font from {}", file.generic_string());
        }
    }

    // Load Saved Text
    if (fs::exists(g_TextEditorFile)) {
        std::ifstream file{g_TextEditorFile};
        using It = std::istreambuf_iterator<char>;
        std::string file_content{It{file}, It{}};
        g_TextEditorBuffer.assign(file_content);

        try {
            g_CurrentTextBuffer = to_str<str>(g_TextEditorBuffer);
            TextModLexer lexer{g_CurrentTextBuffer};
            TextModParser parser{&lexer};
            g_ProgramRule = ProgramRule::create(parser);
            g_ProgramTextTree = nullptr;
        } catch (const ParsingError& err) {
            g_LatestErrorMessage = err.build_error_message(g_CurrentTextBuffer);
        } catch (const std::exception& err) {
            g_LatestErrorMessage = err.what();
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// | TREE BUILDERS IMPL |
////////////////////////////////////////////////////////////////////////////////

constinit auto TREE_FLAGS = ImGuiTreeNodeFlags_SpanFullWidth;

static void update_selection(const auto& rule) {
    if (ImGui::IsItemToggledOpen() || ImGui::IsItemHovered()) {
        g_CurrentTextSelection = rule.text_region();
        g_SelectionChanged = true;
    }
}

void Visitor::on_enter(const auto& rule) {
    // Tree not open
    if (!stack.empty() && !stack.back().IsTreeOpen) {
        return;
    }

    constexpr auto type = rule.ENUM_TYPE;
    constexpr std::array<ParserRuleKind, 3> proxy_rules{RuleExpression, RulePrimitiveExpr, RuleAssignmentExprList};
    constexpr bool is_proxy_rule = std::find(proxy_rules.begin(), proxy_rules.end(), type) != proxy_rules.end();

    if (g_IgnoreProxyRules && (is_proxy_rule || (!g_KeepProgramRule && type == RuleProgram))) {
        return;
    }

    ImGui::PushID(&rule);
    if (ImGui::TreeNodeEx(rule_name(type).data(), TREE_FLAGS)) {
        stack.emplace_back(&rule, true);
    } else {
        stack.emplace_back(&rule, false);
    }

    if (ImGui::IsItemHovered()) {
        update_selection(rule);
    }

    ImGui::PopID();
};

void Visitor::on_exit(const auto& rule) {
    if (stack.empty() || stack.back().Identifier != &rule) {
        return;
    }

    if (stack.back().IsTreeOpen) {
        ImGui::TreePop();
    }

    stack.pop_back();
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// | MAIN |
////////////////////////////////////////////////////////////////////////////////

int main() {
    int code = initialise();

    if (code != 0) {
        LOG_THAT_SHIT("Failed to initialise...");
        return -1;
    }

    while (g_Running) {
        try {
            g_Running = (glfwWindowShouldClose(g_Window) != GLFW_TRUE);
            glfwPollEvents();

            // Start ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport();
            ImGui::ShowDemoWindow();
            update();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(g_Window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(g_Window);
        } catch (const std::exception& err) {
            g_Running = false;
            LOG_THAT_SHIT("[EXCEPTION] {}", err.what());
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(g_Window);
    glfwTerminate();
}