//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "object_explorer.h"
#include "imgui_internal.h"

namespace object_explorer {

namespace {

using TreeNodeBuilder = void (*)(UObject*, UProperty*);

// We can't trust any cached data to stay valid for any amount of time so a little more info is
// required to validate the pointer.
struct GObjects_Query {
    size_t Index;          // Index inside GObjects
    UObject* Obj;          // The pointer when we accessed it
    std::string PathName;  // PathName for the object

    operator bool() const noexcept {
        const GObjects& objs = gobjects();
        return (Index < objs.size()) && (Obj == objs.obj_at(Index));
    }
};

enum EQueryStrategy : uint8_t { STARTS_WITH, ENDS_WITH, EQUALS, CONTAINS };

// This will persist even if the system is restarted so everything in this struct should be
// sanitised or used with the instability in mind.
struct RuntimeConfig {
    // Selection
    size_t GObjects_SelectedIndex = 0;
    UObject* GObjects_SelectedObject = nullptr;
    size_t GObjects_VisibleItemsCount = 0;

    // Queries
    std::vector<GObjects_Query> GObjects_QueryObjects{};
    std::string GObjects_ObjectQuery{};
    std::wstring GObjects_ObjectQueryWide{};
    float GObjects_ObjectQueryDelta{};
    EQueryStrategy GObjects_ObjectQueryStrategy = STARTS_WITH;

    // This is lazily built as new classes are found/discovered
    std::unordered_map<UClass*, TreeNodeBuilder> ObjectTree_TreeNodeBuilderMap{};
    std::set<std::string> ObjectTree_DefaultedTypes{};

    // Retains the last 8 error messages
    std::array<std::string, 8> Error_RingBuffer = {};
    uint8_t Error_Index = 0;

} cfg;

void show_gobjects_list(void);
void show_gobjects_selection(void);
void show_theworld_objects(void);
void show_debug_info(void);

// Builds a complex tree structure for the provided UObject; Lazily builds open nodes
void show_editable_uobject_tree(UObject*);

void show_query_controls(void);
void update_query_objects(void);

}  // namespace

void update() noexcept {
    ImGui::ShowDemoWindow();

    ImGuiErrorRecoveryState state;

#define TRY_SHOW(expr)                          \
    do {                                        \
        ImGui::ErrorRecoveryStoreState(&state); \
        expr();                                 \
    } while (false);

    try {
        TRY_SHOW(show_gobjects_list);
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0F);
        TRY_SHOW(show_gobjects_selection);
        ImGui::PopStyleVar();
        TRY_SHOW(show_theworld_objects);
        TRY_SHOW(show_debug_info);

        //
    } catch (const std::exception& err) {
        uint8_t index = (cfg.Error_Index + 1) % cfg.Error_RingBuffer.size();
        cfg.Error_Index = index;
        cfg.Error_RingBuffer[index] = std::string{err.what()};
        ImGui::ErrorRecoveryTryToRecoverState(&state);

    } catch (...) {
        ImGui::ErrorRecoveryTryToRecoverState(&state);
    }
}

// ############################################################################//
//  | UI STUFF |
// ############################################################################//

namespace {

void show_gobjects_list(void) {
    if (!ImGui::Begin("GObjects")) {
        ImGui::End();
        return;
    }

    show_query_controls();

    const std::vector<GObjects_Query>& objects = cfg.GObjects_QueryObjects;

    if (ImGui::BeginChild("GObjects##QueryList", ImGui::GetContentRegionAvail())) {
        ImGuiListClipper clipper{};
        clipper.Begin(objects.size());
        cfg.GObjects_VisibleItemsCount = 0;

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                cfg.GObjects_VisibleItemsCount++;

                const GObjects_Query& query = objects[i];

                // Still show that it was there but this object is no longer valid
                if (!query) {
                    ImGui::PushID(i);
                    ImVec4 red{1.0F, 0.0F, 0.0F, 1.0F};
                    ImGui::TextColored(red, query.PathName.c_str());
                    ImGui::PopID();
                    continue;
                }

                bool is_selected = static_cast<int>(cfg.GObjects_SelectedIndex) == i;

                ImGui::PushID(i);
                if (ImGui::Selectable(query.PathName.c_str(), is_selected)) {
                    cfg.GObjects_SelectedIndex = query.Index;
                    cfg.GObjects_SelectedObject = query.Obj;
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

// ############################################################################//
//  | OBJECT QUERY |
// ############################################################################//

void show_query_controls(void) {
    auto child_window_flags =
        ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_Borders;
    if (!ImGui::BeginChild("GObjects List Controls", {0, 0}, child_window_flags)) {
        ImGui::EndChild();
        return;
    }

    // Input Text
    std::string* ptr = &cfg.GObjects_ObjectQuery;
    auto text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (ImGui::InputText("Query##GObjects_ObjectQuery", ptr, text_flags)) {
        cfg.GObjects_ObjectQueryWide = str_to_wstr(cfg.GObjects_ObjectQuery);
        update_query_objects();
    }

    // Query Delta
    ImGui::SameLine();
    ImGui::Text("Delta %.2f ms", cfg.GObjects_ObjectQueryDelta * 1000.0F);
    ImGui::SameLine();

    // Query Strategy
    const char* strategies[]{"Starts With", "Ends With", "Equals", "Contains"};
    constexpr int count = sizeof(strategies) / sizeof(strategies[0]);
    const char* label = "##GObjectsQueryStrategy";

    if (ImGui::BeginCombo(label, strategies[cfg.GObjects_ObjectQueryStrategy])) {
        for (uint8_t i = 0; i < count; ++i) {
            bool is_selection = (cfg.GObjects_ObjectQueryStrategy == i);
            auto flags = is_selection ? ImGuiSelectableFlags_Disabled : 0;
            if (ImGui::Selectable(strategies[i], is_selection, flags)) {
                cfg.GObjects_ObjectQueryStrategy = (EQueryStrategy)i;
                update_query_objects();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::EndChild();
}

void update_query_objects(void) {
    auto start = Clock::now();
    std::vector<GObjects_Query>& objects = cfg.GObjects_QueryObjects;
    objects.clear();
    const GObjects& all_objects = gobjects();

    // Query Strategy
    auto pred = [](const std::wstring& path_name) {
        switch (cfg.GObjects_ObjectQueryStrategy) {
            case STARTS_WITH:
                return path_name.starts_with(cfg.GObjects_ObjectQueryWide);
            case ENDS_WITH:
                return path_name.ends_with(cfg.GObjects_ObjectQueryWide);
            case EQUALS:
                return path_name == cfg.GObjects_ObjectQueryWide;
            case CONTAINS:
                return path_name.find(cfg.GObjects_ObjectQueryWide) != std::wstring::npos;
        }
        return false;
    };

    Instant before_query = Clock::now();
    for (size_t i = 0; i < all_objects.size(); ++i) {
        UObject* obj = all_objects.obj_at(i);
        if (!obj) {
            continue;
        }

        std::wstring path_name = obj->get_path_name();
        if (!pred(path_name)) {
            continue;
        }

        objects.emplace_back(i, obj, wstr_to_str(path_name));
    }
    objects.shrink_to_fit();

    cfg.GObjects_ObjectQueryDelta = FloatDuration{Clock::now() - start}.count();
}

// ############################################################################//
//  | OBJECT SELECTION |
// ############################################################################//

void show_gobjects_selection(void) {
    if (!ImGui::Begin("GObject Selection")) {
        ImGui::End();
        return;
    }

    const GObjects& objects = gobjects();
    size_t index = cfg.GObjects_SelectedIndex;

    if (index >= objects.size()) {
        ImGui::Text("Object Selection Invalid!");
        return;
    }

    UObject* obj = gobjects().obj_at(cfg.GObjects_SelectedIndex);

    if (!obj) {
        ImGui::Text("Object Selection Invalid!");
        return;
    }

    show_editable_uobject_tree(obj);

    ImGui::End();
}

// ############################################################################//
//  | THE WORLD OBJECTS |
// ############################################################################//

void show_theworld_objects(void) {
    if (ImGui::Begin("The World")) {
        ImGui::Text("Objects");
    }
    ImGui::End();
    return;
}

// ############################################################################//
//  | DEBUG INFO |
// ############################################################################//

void show_debug_info(void) {
    if (!ImGui::Begin("Object Explorer")) {
        ImGui::End();
        return;
    }

    if (ImGui::TreeNodeEx("Runtime Config", ImGuiTreeNodeFlags_Framed)) {
        ImGui::SeparatorText("GObjects");
        ImGui::Text("GObjects_SelectedIndex     %zu", cfg.GObjects_SelectedIndex);
        ImGui::Text("GObjects_SelectedObject    %p", cfg.GObjects_SelectedObject);
        ImGui::Text("GObjects_VisibleItemsCount %zu", cfg.GObjects_VisibleItemsCount);
        ImGui::Text("GObjects_TotalUObjects     %zu", gobjects().size());

        if (ImGui::TreeNode("ObjectTree TreeNodeBuilderMap")) {
            for (const auto& [key, _] : cfg.ObjectTree_TreeNodeBuilderMap) {
                std::string name{key->Name};
                ImGui::BulletText("%s", name.c_str());
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Object Tree Defaulted Types")) {
            for (std::string cls : cfg.ObjectTree_DefaultedTypes) {
                ImGui::BulletText("%s", cls.c_str());
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Errors")) {
            for (uint8_t i = 0; i < cfg.Error_RingBuffer.size(); ++i) {
                ImGui::TextColored(
                    ImVec4{1.0F, 0.0F, 0.0F, 1.0F},
                    "[%d] - '%s'",
                    i,
                    cfg.Error_RingBuffer[i].c_str()
                );
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

    ImGui::End();
}

}  // namespace

namespace {

// ############################################################################//
//  | UTILITY |
// ############################################################################//

template <class T>
PropTraits<T>::Value get_property(UProperty* prop, UObject* obj, size_t index = 0) {
    return get_property<T>(reinterpret_cast<T*>(prop), index, reinterpret_cast<uintptr_t>(obj));
};

template <class T, class ValueType = PropTraits<T>::Value>
void set_property(UProperty* prop, UObject* obj, ValueType value, size_t index = 0) {
    set_property<T>(reinterpret_cast<T*>(prop), index, reinterpret_cast<uintptr_t>(obj), value);
}

std::string get_unique_label(UObject* obj, UProperty* prop) {
    return std::format(
        "{:p}_{:p}_{}_{}-{}-{}",
        (void*)obj,
        (void*)prop,
        std::string{prop->Name},
        prop->PropertyFlags,
        prop->ArrayDim,
        prop->Offset_Internal
    );
}

// ############################################################################//
//  | TREE BUILDERS |
// ############################################################################//

// Builds the appropriate tree for the given property for the object
void tree_node_builder_object_property(UObject*, UProperty*);
void tree_node_builder_struct_property(UObject*, UProperty*);

// Primitives
void tree_node_builder_int_property(UObject*, UProperty*);
void tree_node_builder_float_property(UObject*, UProperty*);
void tree_node_builder_byte_property(UObject*, UProperty*);
void tree_node_builder_bool_property(UObject*, UProperty*);

// Strings
void tree_node_builder_str_property(UObject*, UProperty*);
void tree_node_builder_name_property(UObject*, UProperty*);

// Default
void tree_node_builder_default(UObject*, UProperty*);

TreeNodeBuilder find_or_create_parser(UClass* cls) {
    auto& tree_node_builder_map = cfg.ObjectTree_TreeNodeBuilderMap;

    if (!tree_node_builder_map.empty()) [[likely]] {
        auto it = tree_node_builder_map.find(cls);
        if (it != tree_node_builder_map.end()) {
            return it->second;
        }
        return &tree_node_builder_default;
    }

    auto insert_builder = [](std::wstring_view cls_path_name, TreeNodeBuilder builder) {
        UClass* cls = find_class(cls_path_name);
        if (!cls) {
            LOG(DEV_WARNING, L"Failed to find class for '{}'", cls_path_name);
            return;
        }
        cfg.ObjectTree_TreeNodeBuilderMap[cls] = builder;
    };

    // Composite
    insert_builder(L"Core.ObjectProperty", &tree_node_builder_object_property);
    insert_builder(L"Core.StructProperty", &tree_node_builder_struct_property);

    // Leafs
    insert_builder(L"Core.IntProperty", &tree_node_builder_int_property);
    insert_builder(L"Core.FloatProperty", &tree_node_builder_float_property);
    insert_builder(L"Core.ByteProperty", &tree_node_builder_byte_property);
    insert_builder(L"Core.BoolProperty", &tree_node_builder_bool_property);

    insert_builder(L"Core.StrProperty", &tree_node_builder_str_property);
    insert_builder(L"Core.NameProperty", &tree_node_builder_name_property);

    cfg.ObjectTree_DefaultedTypes.insert(wstr_to_str(cls->get_path_name()));
    return find_or_create_parser(cls);
}

void show_editable_uobject_tree(UObject* obj) {
    tree_node_builder_object_property(obj, nullptr);
}

// ############################################################################//
//  | TREE NODE BUILDERS |
// ############################################################################//

void tree_node_builder_default(UObject* obj, UProperty* in_prop) {
    std::string label = std::format("{}, {}", obj->Name, in_prop->get_path_name());
    ImGui::Text(label.c_str());
}

void tree_node_builder_object_property(UObject* obj_in, UProperty* obj_prop) {
    // TODO: This one is extra messy

    if (!obj_in || !obj_in->Class) {
        if (obj_prop) {
            std::string prop_name = std::format("Object is NULL for property {}", obj_prop->Name);
            ImGui::Text(prop_name.c_str());
        } else {
            ImGui::Text("Object Property is NULL");
        }
        return;
    }

    UObject* obj = obj_in;
    bool has_tree_open = false;

    // If given a valid property then display
    if (obj_prop) {
        obj = get_property<UObjectProperty>(obj_prop, obj_in);

        // Object stored in the property is null
        if (!obj) {
            std::string text = std::format("{} is NULL", obj_prop->Name);
            ImGui::Text(text.c_str());
            return;
        }

        std::string label = std::format("{}: {}##{:p}", obj_prop->Name, obj->Name, (void*)obj);
        if (ImGui::TreeNodeEx(label.c_str())) {
            has_tree_open = true;
        } else {
            return;
        }
    }

    // Meta
    std::string path_name = wstr_to_str(obj->get_path_name());
    ImGui::Text("Path Name '%s'", path_name.c_str());

    std::string obj_name = obj->Name;
    ImGui::Text("Name '%s'", obj_name.c_str());

    if (obj->Outer) {
        std::string outer_label =
            std::format("Outer '{}'##{:p}_{:p}", obj->Outer->Name, (void*)obj, (void*)obj->Outer);
        if (ImGui::TreeNodeEx(outer_label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            tree_node_builder_object_property(obj->Outer, nullptr);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("Outer 'NULL'");
    }

    if (obj->Class) {
        std::string class_label =
            std::format("Class '{}'##{:p}_{:p}", obj->Class->Name, (void*)obj, (void*)obj->Class);
        if (ImGui::TreeNodeEx(class_label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            tree_node_builder_object_property(obj->Class, nullptr);
            ImGui::TreePop();
        }
    } else {
        ImGui::Text("Class 'NULL'");
    }

    std::string flags = std::format("Object Flags 0x{:X}", obj->ObjectFlags);
    ImGui::Text(flags.c_str());

    std::string properties_label =
        std::format("Object Properties##_{:p}_{}_{:p}", (void*)obj, obj->Name, (void*)obj_prop);
    if (ImGui::TreeNodeEx(properties_label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
        for (UProperty* prop : obj->Class->properties()) {
            TreeNodeBuilder builder = find_or_create_parser(prop->Class);
            builder(obj, prop);
        }
        ImGui::TreePop();
    }

    if (has_tree_open) {
        ImGui::TreePop();
    }
}

void tree_node_builder_struct_property(UObject* obj, UProperty* prop) {
    UStructProperty* struct_prop = reinterpret_cast<UStructProperty*>(prop);
    UScriptStruct* inner = struct_prop->get_inner_struct();

    std::string label = std::format(
        "{}##StructProp_{}",
        prop->Name,
        get_unique_label(obj, prop)
    );

    bool is_open = ImGui::TreeNode(label.c_str());

    if (inner && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::BeginTooltip();
        std::string inner_name = std::string{inner->Name};
        ImGui::TextWrapped(inner_name.c_str());
        ImGui::EndTooltip();
    }

    if (is_open) {
        if (!inner) {
            ImGui::Text("Inner Struct is NULL");
            ImGui::TreePop();
            return;
        }

        std::string struct_meta_label = "Meta Info##" + label;
        if (ImGui::TreeNodeEx(struct_meta_label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            std::string inner_name = inner->Name;
            ImGui::Text("Struct Name %s", inner_name.c_str());

            std::string prop_size = std::format("Prop Size 0x{:X}", prop->ElementSize);
            ImGui::Text(prop_size.c_str());

            std::string inner_prop_size =
                std::format("Inner Prop Size 0x{:X}", inner->PropertySize);
            ImGui::Text(inner_prop_size.c_str());

            std::string flags = std::format("Struct Flags 0x{:X}", obj->ObjectFlags);
            ImGui::Text(flags.c_str());
            ImGui::TreePop();
        }

        for (UProperty* inner_prop : inner->properties()) {
            TreeNodeBuilder builder = find_or_create_parser(inner_prop->Class);
            builder(obj, inner_prop);
        }
        ImGui::TreePop();
    }
}

void tree_node_builder_int_property(UObject* obj, UProperty* prop) {
    constexpr auto limits = std::numeric_limits<int16_t>{};
    constexpr int32_t MIN = static_cast<int32_t>(limits.min());
    constexpr int32_t MAX = static_cast<int32_t>(limits.max());
    int32_t value = get_property<UIntProperty>(prop, obj);

    std::string label = std::format("{}##IntProp_S{}", prop->Name, get_unique_label(obj, prop));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.33F);
    if (ImGui::SliderInt(label.c_str(), &value, MIN, MAX, "%d")) {
        set_property<UIntProperty>(prop, obj, value);
    }
}

void tree_node_builder_float_property(UObject* obj, UProperty* prop) {
    constexpr auto limits = std::numeric_limits<int16_t>{};
    constexpr float MIN = static_cast<float32_t>(limits.min());
    constexpr float MAX = static_cast<float32_t>(limits.max());
    constexpr auto flags = ImGuiSliderFlags_NoRoundToFormat;
    float32_t value = get_property<UFloatProperty>(prop, obj);

    std::string label = std::format("{}##FloatProp_S{}", prop->Name, get_unique_label(obj, prop));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.33F);
    if (ImGui::SliderFloat(label.c_str(), &value, MIN, MAX, "%.6f", flags)) {
        set_property<UFloatProperty>(prop, obj, value);
    }
}

void tree_node_builder_byte_property(UObject* obj, UProperty* prop) {
    constexpr auto limits = std::numeric_limits<uint8_t>{};
    constexpr int MIN = static_cast<int>(limits.min());
    constexpr int MAX = static_cast<int>(limits.max());

    int value = static_cast<int>(get_property<UByteProperty>(prop, obj));

    std::string label = std::format("{}##ByteProp_S{}", prop->Name, get_unique_label(obj, prop));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.33F);
    if (ImGui::SliderInt(label.c_str(), &value, MIN, MAX, "%d")) {
        set_property<UByteProperty>(prop, obj, static_cast<uint8_t>(value));
    }
}

void tree_node_builder_bool_property(UObject* obj, UProperty* prop) {
    bool value = get_property<UBoolProperty>(prop, obj);
    std::string label = std::format("{}##BoolProp_{}", prop->Name, get_unique_label(obj, prop));

    if (ImGui::Checkbox(label.c_str(), &value)) {
        set_property<UBoolProperty>(prop, obj, value);
    }
}

void tree_node_builder_str_property(UObject* obj, UProperty* prop) {
    std::string label = std::format("{}##StrProp_{}", prop->Name, get_unique_label(obj, prop));
    if (ImGui::TreeNode(label.c_str())) {
        std::string value = wstr_to_str(get_property<UStrProperty>(prop, obj));
        ImGui::Text(value.c_str());
        ImGui::TreePop();
    }
}

void tree_node_builder_name_property(UObject* obj, UProperty* prop) {
    std::string label = std::format("{}##NameProp_{}", prop->Name, get_unique_label(obj, prop));

    if (ImGui::TreeNode(label.c_str())) {
        FName name = get_property<UNameProperty>(prop, obj);

        std::string value = std::format("Value '{}'", name);
        ImGui::Text(value.c_str());

        static_assert(sizeof(FName) == sizeof(int32_t) * 2);
        int32_t* hack = reinterpret_cast<int32_t*>(&name);
        ImGui::Text("Index  %d", hack[0]);
        ImGui::Text("Number %d", hack[1]);

        ImGui::TreePop();
    }
}

}  // namespace
}  // namespace object_explorer