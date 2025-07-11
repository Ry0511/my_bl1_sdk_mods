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

#include "tm_parse/common/text_mod_common.h"
#include "tm_parse/lexer/text_mod_lexer.h"
#include "tm_parse/parser/text_mod_parser.h"

#define LOG_THAT_SHIT(...) __VA_OPT__(std::cout << std::format(__VA_ARGS__) << std::endl)

////////////////////////////////////////////////////////////////////////////////
// | GLOBALS |
////////////////////////////////////////////////////////////////////////////////

namespace {

using namespace tm_parse;

ImGuiContext* g_Context = nullptr;
ImFont* g_CascadiaCodeFont = nullptr;
GLFWwindow* g_Window = nullptr;
bool g_Running = true;

std::unique_ptr<fs::path> g_CurrentFile{};
ProgramRule g_ProgramRule{};
std::unique_ptr<std::string> g_ProgramTextTree{};

fs::path g_TextEditorFile = fs::current_path() / L"text_editor_content.txt";
std::string g_TextEditorBuffer{};

TokenTextView g_CurrentTextSelection{};
TokenTextView g_PreviousTextSelection{};

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

void update() {
    _text_editor();
    _tree_view();
    _tree_text_view();
}

static void _text_editor() {
    if (!ImGui::Begin(UID("Text Editor"))) {
        ImGui::End();
        return;
    }

    auto callback = [](ImGuiInputTextCallbackData* data) -> int {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
            g_TextEditorBuffer.resize(data->BufTextLen);
            data->Buf = g_TextEditorBuffer.data();
        }
        return 0;
    };

    ImGui::SetKeyboardFocusHere();
    ImGui::InputTextMultiline(
        UID("Text Editor"),
        g_TextEditorBuffer.data(),
        g_TextEditorBuffer.size(),
        ImGui::GetContentRegionAvail(),
        ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AutoSelectAll,
        callback
    );

    if (g_CurrentTextSelection.is_valid()) {
        set_selection(g_CurrentTextSelection.Start, g_CurrentTextSelection.end());
    }

    if (ImGui::IsItemClicked()) {
        g_CurrentTextSelection= {};
    }

    if (ImGui::IsWindowFocused() && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        try {
            TextModLexer lexer{g_TextEditorBuffer};
            TextModParser parser{&lexer};
            g_ProgramRule = ProgramRule::create(parser);
            g_ProgramTextTree = nullptr;

        } catch (const std::exception& err) {
            LOG_THAT_SHIT("{}", err.what());
        }

        try {
            std::ofstream file{g_TextEditorFile};
            file << g_TextEditorBuffer;
        } catch (const std::exception& err) {
            LOG_THAT_SHIT("Failed to save file: {}", err.what());
        }
    }

    ImGui::End();
}

template <class T>
static void _build_tree(const T& rule) = delete;

#define TREE_BUILDER(type) \
    template <>            \
    static void _build_tree(const type& rule);

// primary_rules.h
TREE_BUILDER(ProgramRule);
TREE_BUILDER(ObjectDefinitionRule);
TREE_BUILDER(SetCommandRule);

// primary_expr_rules.h
TREE_BUILDER(AssignmentExprRule);
TREE_BUILDER(AssignmentExprListRule);
TREE_BUILDER(ParenExprRule);
TREE_BUILDER(ExpressionRule);

// primitive_expr_rules.h
TREE_BUILDER(PrimitiveExprRule);
TREE_BUILDER(NumberExprRule);
TREE_BUILDER(StrExprRule);
TREE_BUILDER(NameExprRule);
TREE_BUILDER(KeywordRule);
TREE_BUILDER(LiteralExprRule);

// parser_rules.h
TREE_BUILDER(IdentifierRule);
TREE_BUILDER(DotIdentifierRule);
TREE_BUILDER(ObjectIdentifierRule);
TREE_BUILDER(ObjectAccessRule);
TREE_BUILDER(ArrayAccessRule);
TREE_BUILDER(PropertyAccessRule);

template <>
static void _build_tree(const std::monostate&) {}

#undef TREE_BUILDER

static void _tree_view() {
    if (!ImGui::Begin(UID("Tree View"))) {
        ImGui::End();
        return;
    }

    if (!g_ProgramRule.operator bool()) {
        ImGui::Text("Program rule is invalid");
    } else {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0F);
        _build_tree(g_ProgramRule);
        ImGui::PopStyleVar();
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
        g_TextEditorBuffer.assign(It{file}, It{});

        try {
            TextModLexer lexer{g_TextEditorBuffer};
            TextModParser parser{&lexer};
            g_ProgramRule = ProgramRule::create(parser);
            g_ProgramTextTree = nullptr;

        } catch (const std::exception& err) {
            LOG_THAT_SHIT("{}", err.what());
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// | TREE BUILDERS IMPL |
////////////////////////////////////////////////////////////////////////////////

constexpr auto TREE_FLAGS = ImGuiTreeNodeFlags_SpanAvailWidth;

static void update_selection(const auto& rule) {
    if (ImGui::IsItemToggledOpen()) {
        g_CurrentTextSelection = rule.text_region();
        LOG_THAT_SHIT("Selection Updated: {}, {}", g_CurrentTextSelection.Start, g_CurrentTextSelection.end());
    }
}

template <>
static void _build_tree(const ProgramRule& rule) {
    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx("ProgramRule", TREE_FLAGS)) {
        ImGui::PopID();
        return;
    }

    for (const auto& inner : rule.rules()) {
        std::visit([](const auto& inner) -> void { _build_tree(inner); }, inner);
    }

    ImGui::TreePop();
    ImGui::PopID();
}

template <>
static void _build_tree(const SetCommandRule& rule) {
    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx("SetCommand", ImGuiTreeNodeFlags_SpanAvailWidth)) {
        ImGui::PopID();
        return;
    }

    update_selection(rule);
    _build_tree(rule.object());
    _build_tree(rule.property());
    _build_tree(rule.expr());

    ImGui::TreePop();
    ImGui::PopID();
}

template <>
static void _build_tree(const ObjectDefinitionRule& rule) {
    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx("ObjectDefinition", TREE_FLAGS)) {
        ImGui::PopID();
        return;
    }

    update_selection(rule);

    for (const auto& inner : rule.child_objects()) {
        _build_tree(*inner);
    }

    for (const auto& inner : rule.assignments()) {
        _build_tree(inner);
    }

    ImGui::TreePop();
    ImGui::PopID();
}

template <>
static void _build_tree(const AssignmentExprListRule& rule) {
    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx("AssignmentExprList", TREE_FLAGS)) {
        ImGui::PopID();
        return;
    }

    update_selection(rule);

    for (const AssignmentExprRule& inner : rule) {
        _build_tree(inner);
    }

    ImGui::TreePop();
    ImGui::PopID();
}

template <>
static void _build_tree(const AssignmentExprRule& rule) {
    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx("AssignmentExpr", TREE_FLAGS)) {
        ImGui::PopID();
        return;
    }

    update_selection(rule);
    _build_tree(rule.property());

    if (rule.has_expr()) {
        _build_tree(rule.expr());
    }

    ImGui::TreePop();
    ImGui::PopID();
}

template <>
static void _build_tree(const ObjectAccessRule& rule) {
    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx("ObjectAccess", TREE_FLAGS)) {
        ImGui::PopID();
        return;
    }

    update_selection(rule);
    if (const auto& cls_type = rule.class_type()) {
        _build_tree(cls_type);
    }

    _build_tree(rule.object_path());

    ImGui::TreePop();
    ImGui::PopID();
}

template <>
static void _build_tree(const PropertyAccessRule& rule) {
    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx("PropertyAccess", TREE_FLAGS)) {
        ImGui::PopID();
        return;
    }

    update_selection(rule);
    _build_tree(rule.identifier());

    if (const auto& array_access = rule.array_access()) {
        _build_tree(array_access);
    }

    ImGui::TreePop();
    ImGui::PopID();
}

template <>
static void _build_tree(const ExpressionRule& rule) {
    if (rule.operator bool()) {
        std::visit([](const auto& inner) { _build_tree(inner); }, rule.inner());
    } else {
        ImGui::PushID(&rule);
        ImGui::TreeNodeEx("Expression", TREE_FLAGS | ImGuiTreeNodeFlags_Leaf);
        ImGui::TreePop();
        ImGui::PopID();
    }
}

template <>
static void _build_tree(const ParenExprRule& rule) {
    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx("ParenExpr", TREE_FLAGS)) {
        ImGui::PopID();
        return;
    }

    update_selection(rule);
    if (auto* inner = rule.inner_most()) {
        _build_tree(*inner);
    }
    ImGui::TreePop();
    ImGui::PopID();
}

template <>
static void _build_tree(const PrimitiveExprRule& rule) {
    if (rule.operator bool()) {
        std::visit([](const auto& inner) { _build_tree(inner); }, rule.inner());
    } else {
        ImGui::PushID(&rule);
        ImGui::TreeNodeEx("PrimitiveExpr", TREE_FLAGS | ImGuiTreeNodeFlags_Leaf);
        ImGui::TreePop();
        ImGui::PopID();
    }
}

template<>
static void _build_tree(const ObjectIdentifierRule& rule) {
    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx("ObjectIdentifier", TREE_FLAGS)) {
        ImGui::PopID();
        return;
    }

    update_selection(rule);
    _build_tree(rule.primary_identifier());

    if (const auto& child = rule.child_identifier()) {
        _build_tree(child);
    }

    ImGui::TreePop();
    ImGui::PopID();
}

static void _build_tree_leaf(const auto& rule) {
    using T = std::decay_t<decltype(rule)>;
    str kind = str{rule_name(T::ENUM_TYPE)};

    ImGui::PushID(&rule);
    if (!ImGui::TreeNodeEx(kind.c_str(), TREE_FLAGS)) {
        ImGui::PopID();
        return;
    }

    update_selection(rule);
    ImGui::TreePop();
    ImGui::PopID();
}

#define LEAF_NODE_IMPL(type)                    \
    template <>                                 \
    static void _build_tree(const type& rule) { \
        _build_tree_leaf(rule);                 \
    }

LEAF_NODE_IMPL(NumberExprRule);
LEAF_NODE_IMPL(StrExprRule);
LEAF_NODE_IMPL(NameExprRule);
LEAF_NODE_IMPL(KeywordRule);
LEAF_NODE_IMPL(LiteralExprRule);

LEAF_NODE_IMPL(IdentifierRule);
LEAF_NODE_IMPL(DotIdentifierRule);
LEAF_NODE_IMPL(ArrayAccessRule);

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