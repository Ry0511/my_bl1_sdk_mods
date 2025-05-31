//
// Date       : 01/12/2024
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pyunrealsdk/pch.h"
#include "unrealsdk/pch.h"

#include "object_explorer.h"

#include "unrealsdk/memory.h"
#include "unrealsdk/unrealsdk.h"

#include <unrealsdk/unreal/classes/properties/uobjectproperty.h>
#include <unrealsdk/unreal/classes/uscriptstruct.h>
#include <unrealsdk/unreal/structs/fstring.h>
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

PYBIND11_MODULE(object_explorer, m) {
    m.def("initialise", []() {
        LOG(INFO, "Initialising object explorer...");
        object_explorer::initialise();
    });

    m.def("terminate", []() {
        LOG(INFO, "Terminating object explorer...");
        object_explorer::shutdown();
    });


    // TODO: Remove everything after this point
    using namespace unrealsdk;
    using namespace unrealsdk::unreal;
    using namespace unrealsdk::memory;

    // constexpr Pattern<28> export_text_func{
    //     "F6 44 24 18 80 56 8B F1 74 12 8B 06 8B 90 84 01 00 00 FF D2 85 C0 75 04 5E C2 18 00"
    // };
    // // NOLINTNEXTLINE(*-use-using)
    // typedef bool(__fastcall * export_text_fn)(
    //     UProperty * ecx,
    //     void* edx,
    //     int32_t index,
    //     ManagedFString& str,
    //     uint8_t* data,
    //     uint8_t* delta,
    //     UObject* parent,
    //     int32_t port_flags
    // );
    //
    // static export_text_fn export_text = nullptr;
    // export_text = reinterpret_cast<export_text_fn>(export_text_func.sigscan_nullable<export_text_fn>());
    //
    // auto export_text_impl = [](UObject* obj) -> std::optional<std::string> {
    //     if (export_text == nullptr) {
    //         LOG(INFO, "Export text function not found.");
    //         return std::nullopt;
    //     }
    //
    //     std::wstringstream out{};
    //
    //     auto dump_property = [](std::wstringstream& out, UObject* obj, UProperty* prop) -> void {
    //         auto base_addr = reinterpret_cast<uintptr_t>(obj);
    //         auto obj_data = reinterpret_cast<uint8_t*>(obj);
    //
    //         constexpr auto ppf_flags = 0x20001;  // yoinked from ghidra
    //
    //         ManagedFString obj_str{};
    //         obj_str.reserve(128);
    //
    //         // gets inner property name if there is one
    //         auto get_prop_type = [](UProperty* prop) -> std::wstring {
    //             if (prop->Class->Name == L"StructProperty"_fn) {
    //                 auto* inner = reinterpret_cast<UStructProperty*>(prop)->get_inner_struct();
    //                 return std::wstring{inner->Name} + L' ';
    //             }
    //
    //             if (prop->Class->Name == L"ObjectProperty"_fn) {
    //                 auto* pclass = reinterpret_cast<UObjectProperty*>(prop)->get_property_class();
    //                 return std::wstring{pclass->Name} + L' ';
    //             }
    //
    //             if (prop->Class->Name == L"ArrayProperty"_fn) {
    //                 auto* inner = reinterpret_cast<UArrayProperty*>(prop)->get_inner();
    //                 return std::wstring{inner->Class->Name} + L' ';
    //             }
    //
    //             return std::wstring{L"_Core_ "};
    //         };
    //
    //         // Inplace arrays
    //         if (prop->ArrayDim > 1) {
    //             for (int32_t i = 0; i < prop->ArrayDim; ++i) {
    //                 std::memset(obj_str.data, L'\0', obj_str.capacity());
    //                 obj_str.resize(0);
    //
    //                 // Array of this property
    //                 out << prop->Class->Name << L' ' << get_prop_type(prop);
    //
    //                 // NOLINTNEXTLINE(*)
    //                 if (export_text(prop, nullptr, i, obj_str, obj_data, obj_data, obj, ppf_flags)) {
    //                     out << prop->Name << L"[" << i << L"]=" << obj_str.data << L'\n';
    //                 } else {
    //                     out << prop->Name << L"[" << i << L"]=__ERROR_IN_PARSING__\n";
    //                 }
    //             }
    //
    //             return;
    //         }
    //
    //         // UArray Properties
    //         if (prop->Class->Name == L"ArrayProperty"_fn) {
    //             auto* array_prop = reinterpret_cast<UArrayProperty*>(prop);
    //
    //             const auto* inner = array_prop->get_inner();
    //             const WrappedArray arr = get_property(array_prop, 0, base_addr);
    //             const auto elem_size = inner->ElementSize;
    //
    //             for (size_t i = 0; i < arr.size(); ++i) {
    //                 std::memset(obj_str.data, L'\0', obj_str.capacity());
    //                 obj_str.resize(0);
    //
    //                 auto addr = reinterpret_cast<uintptr_t>(arr.base->data) + (i * elem_size);
    //                 auto* data = reinterpret_cast<uint8_t*>(addr);
    //
    //                 out << prop->Class->Name << L' ' << get_prop_type(prop);
    //
    //                 // NOLINTNEXTLINE(*)
    //                 bool ok = inner->call_virtual_function<bool>(0x53, &obj_str, data, data, obj, ppf_flags);
    //
    //                 if (ok) {
    //                     out << prop->Name << L"(" << i << L")=" << obj_str.data << L'\n';
    //                 } else {
    //                     out << prop->Name << L"(" << i << L")=__ERROR_IN_PARSING__\n";
    //                 }
    //             }
    //
    //             return;
    //         }
    //
    //         out << prop->Class->Name << L' ' << get_prop_type(prop);
    //
    //         // NOLINTNEXTLINE(*)
    //         if (export_text(prop, nullptr, 0, obj_str, (uint8_t*)obj, (uint8_t*)obj, obj, ppf_flags)) {
    //             out << prop->Name << L"=" << obj_str.data << L'\n';
    //         } else {
    //             out << prop->Name << L"=__ERROR_IN_PARSING__\n";
    //         }
    //     };
    //
    //     // NOLINTNEXTLINE(*)
    //     for (UProperty* prop : obj->Class->properties()) {
    //         dump_property(out, obj, prop);
    //     }
    //
    //     return utils::narrow(out.str());
    // };
    //
    // m.def("export_text", export_text_impl, "obj"_a);
    //
    // {
    //     std::ofstream object_file_dump{"gobjects_dump.txt", std::ios::out | std::ios::trunc};
    //     const std::string separator(120, '#');
    //
    //     for (UObject* obj : gobjects()) {
    //         if (obj == nullptr) {
    //             continue;
    //         }
    //
    //         if (obj->Class->Name == L"Function"_fn) {
    //             continue;
    //         }
    //
    //         object_file_dump << separator << '\n';
    //         object_file_dump << "# " << utils::narrow(obj->get_path_name()) << " \n";
    //         object_file_dump << separator << '\n';
    //         auto dump = export_text_impl(obj);
    //
    //         if (!dump.has_value()) {
    //             object_file_dump << "Failed to dump object..." << '\n';
    //         } else {
    //             object_file_dump << dump.value() << '\n';
    //         }
    //     }
    //
    //     object_file_dump.close();
    // }
    //
    // // NOLINTBEGIN(*-magic-numbers)
    // constexpr Pattern<23> max_level_func{"F6 81 2E 09 00 00 04 B8 32 00 00 00 74 05 B8 3D 00 00 00 83 C0 08 C3"};
    //
    // // General max level
    // auto* bytes = max_level_func.sigscan_nullable<uint8_t*>();
    // if (bytes != nullptr) {
    //     unlock_range(reinterpret_cast<uintptr_t>(bytes), 23);
    //     std::memset((void*)bytes, 0x90, 23);  // Fill with NOP
    //     bytes[0] = 0xB8;                      // MOV EAX, imm32
    //     auto* max_level = reinterpret_cast<int32_t*>(bytes + 1);
    //     *max_level = 90;
    //     bytes[5] = 0xC3;  // RET
    // }
    //
    // // Weapon max level changer
    // constexpr Pattern<17> weap_max_level_func{"83 F8 47 89 44 24 14 7E 08 C7 44 24 14 47 00 00 00"};
    // bytes = weap_max_level_func.sigscan<uint8_t*>("_oe_weap_max_level_func");
    // if (bytes != nullptr) {
    //     unlock_range(reinterpret_cast<uintptr_t>(bytes), 2);
    //     bytes[2] = 92;
    // }
    //
    // // NOLINTEND(*-magic-numbers)

    m.def(
        "get_addr",
        [](const UFunction* func) -> std::string {
            return fmt::format("{:p}", func->Func);
        },
        "func"_a
    );
}