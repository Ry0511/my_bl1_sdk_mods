//
// Date       : 16/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "text_mods.h"

#include "unrealsdk/memory.h"
#include "unrealsdk/utils.h"

#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/structs/fstring.h"

#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"

#include "text_mod_lexer.h"

using namespace unrealsdk;
using namespace unrealsdk::memory;
using namespace unrealsdk::unreal;

namespace {

////////////////////////////////////////////////////////////////////////////////
// | HELPERS |
////////////////////////////////////////////////////////////////////////////////

// Grabbed from ghidra, probably PPF_Transient and PPF_Localized
constexpr auto dflt_export_flags = 0x20001;

// PPF_Localized
constexpr auto dflt_import_flags = 0x1;

////////////////////////////////////////////////////////////////////////////////
// | IMPORT TEXT |
////////////////////////////////////////////////////////////////////////////////

// ImportText Vtable index
constexpr auto import_text_vidx = 0x54;

bool impl_import_text(UObject* obj, UProperty* prop, const wchar_t* const txt) {
    // TODO: Object changed events

    auto base_addr = reinterpret_cast<uintptr_t>(obj) + static_cast<uintptr_t>(prop->Offset_Internal);
    const auto prop_data = reinterpret_cast<uint8_t*>(base_addr);

    // UProperty* ImportText(wchar_t*, void*, uint32_t, UObject*, void*)
    const wchar_t* result =
        prop->call_virtual_function<const wchar_t*>(import_text_vidx, txt, prop_data, dflt_import_flags, obj, nullptr);

    // Think it also returns the final processing position
    return result != nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// | EXPORT TEXT |
////////////////////////////////////////////////////////////////////////////////

// Weird that this is not a vtable function but it seems ExportTextItem is the actual polymorphic variant
constexpr Pattern<28> export_text_func{
    // TODO: Format this
    "F6 44 24 18 80 56 8B F1 74 12 8B 06 8B 90 84 01 00 00 FF D2 85 C0 75 04 5E C2 18 00"
};

// NOLINTNEXTLINE(*-use-using)
typedef bool(__fastcall* export_text_fn)(
    UProperty* ecx,
    void* edx,
    int32_t index,
    ManagedFString& str,
    uint8_t* data,
    uint8_t* delta,
    UObject* parent,
    int32_t port_flags
);

static export_text_fn export_text_ptr = nullptr;

bool impl_export_text(
    UProperty* prop,
    int32_t index,
    ManagedFString& out_str,
    uint8_t* data,
    uint8_t* delta,
    UObject* parent,
    int32_t port_flags = dflt_export_flags
) {
    return export_text_ptr(prop, nullptr, index, out_str, data, delta, parent, port_flags);
}

////////////////////////////////////////////////////////////////////////////////
// | EXPORT TEXT ITEM |
////////////////////////////////////////////////////////////////////////////////

// Vtable index for UProperty::ExportTextItem
constexpr auto export_text_item_vidx = 0x53;

bool impl_export_text_item(
    const UProperty* prop,
    ManagedFString& out_str,
    uint8_t* data,
    uint8_t* delta,
    UObject* parent,
    int32_t port_flags = dflt_export_flags
) {
    return prop->call_virtual_function<bool>(export_text_item_vidx, &out_str, data, delta, parent, port_flags);
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// | PUBLIC INTERFACE |
////////////////////////////////////////////////////////////////////////////////

namespace text_mods {

////////////////////////////////////////////////////////////////////////////////
// | INITIALISE |
////////////////////////////////////////////////////////////////////////////////

void initialise() {
    export_text_ptr = reinterpret_cast<export_text_fn>(export_text_func.sigscan_nullable<export_text_fn>());

    if (export_text_ptr == nullptr) {
        throw std::runtime_error("Failed to find export text function.");
    }

    LOG(INFO, "[text_mods] ExportText function '{:p}'", reinterpret_cast<void*>(export_text_ptr));
}

////////////////////////////////////////////////////////////////////////////////
// | IMPORT TEXT |
////////////////////////////////////////////////////////////////////////////////

bool import_text(UObject* obj, UProperty* prop, std::string_view text) {
    std::wstring cv = utils::widen(text);
    return import_wtext(obj, prop, cv);
}

bool import_wtext(UObject* obj, UProperty* prop, std::wstring_view text) {
    if (obj == nullptr || prop == nullptr) {
        throw std::runtime_error{"Can't import text into a null object/property"};
    }

    return impl_import_text(obj, prop, text.data());
}

void invoke_set_cmd(std::wstring_view cmd, bool keep_alive) {
    (void)cmd, keep_alive;
    throw std::logic_error{"Not implemented"};
}

void import_text_file(const fs::path& txt_file) {
    if (!fs::exists(txt_file) || !fs::is_regular_file(txt_file)) {
        throw std::runtime_error{"File does not exist or is not a regular file"};
    }

    // TODO: Implement
}

////////////////////////////////////////////////////////////////////////////////
// | EXPORT TEXT |
////////////////////////////////////////////////////////////////////////////////

std::wstring export_wtext(UObject* obj) {
    if (obj == nullptr) {
        throw std::runtime_error{"Can't export text from a null object"};
    }

    // TODO: This can and probably should include the child objects attached to the object itself

    using str = std::wstring;

    std::wstringstream out{};
    ManagedFString temp_str{};
    temp_str.reserve(128);

    // Bit of a hack since no reset/clear functionality exists
    auto reset = [&temp_str]() -> void {
        temp_str.resize(0);
        std::memset(temp_str.data, L'\0', temp_str.capacity());
    };

    reset();

    const wchar_t* dflt_err_msg = L"_ERROR_IN_EXPORT_";

    // Main object header; We use the Name but I would prefer the path name tbh
    out << fmt::format(L"Begin Object Class={} Name={}\n", str{obj->Class->Name}, str{obj->Name});

    // Ordering on these is not ideal
    for (UProperty* prop : obj->Class->properties()) {
        ////////////////////////////////////////////////////////////////////////////////
        // | EXPORTING STACK ARRAY |
        ////////////////////////////////////////////////////////////////////////////////

        if (prop->ArrayDim > 1) {
            uint8_t* data = reinterpret_cast<uint8_t*>(obj);

            for (int32_t i = 0; i < prop->ArrayDim; ++i) {
                if (impl_export_text(prop, i, temp_str, data, data, obj)) {
                    out << fmt::format(L"  {}[{}]={}\n", str{prop->Name}, i, temp_str.data);
                } else {
                    out << fmt::format(L"  {}[{}]={}\n", str{prop->Name}, i, dflt_err_msg);
                }
                reset();
            }

            continue;  // Skip outer
        }

        ////////////////////////////////////////////////////////////////////////////////
        // | EXPORTING DYNAMIC ARRAY |
        ////////////////////////////////////////////////////////////////////////////////

        if (prop->Class->Name == FName{ClassTraits<UArrayProperty>::NAME}) {
            auto* array_prop = reinterpret_cast<UArrayProperty*>(prop);
            const auto* inner = array_prop->get_inner();
            const WrappedArray arr = get_property(array_prop, 0, reinterpret_cast<uintptr_t>(obj));
            const auto elem_size = inner->ElementSize;

            for (size_t i = 0; i < arr.size(); ++i) {
                auto addr = reinterpret_cast<uintptr_t>(arr.base->data) + (i * elem_size);
                uint8_t* data = reinterpret_cast<uint8_t*>(addr);

                if (impl_export_text_item(inner, temp_str, data, data, obj)) {
                    out << fmt::format(L"  {}({})={}\n", str{prop->Name}, i, temp_str.data);
                } else {
                    out << fmt::format(L"  {}({})={}\n", str{prop->Name}, i, dflt_err_msg);
                }

                reset();
            }

            continue;  // Skip outer
        }

        ////////////////////////////////////////////////////////////////////////////////
        // | EXPORTING REGULAR PROPERTY |
        ////////////////////////////////////////////////////////////////////////////////

        {
            auto* data = reinterpret_cast<uint8_t*>(obj);
            if (impl_export_text(prop, 0, temp_str, data, data, obj)) {
                out << fmt::format(L"  {}={}\n", str{prop->Name}, temp_str.data);
            } else {
                out << fmt::format(L"  {}={}\n", str{prop->Name}, dflt_err_msg);
            }

            reset();
        }
    }

    out << L"End Object";

    return out.str();
}

std::wstring export_wtext_prop(UObject* obj, UProperty* prop, int32_t index) {
    if (obj == nullptr || prop == nullptr) {
        throw std::runtime_error{"Can't export text from a null object/property"};
    }

    // Default error message
    const wchar_t* dflt_err_msg = L"_ERROR_IN_EXPORT_";

    // Primary defs
    using str = std::wstring;
    std::wstringstream out{};
    ManagedFString temp_str{};
    temp_str.reserve(128);

    ////////////////////////////////////////////////////////////////////////////////
    // | STACK ARRAY PROPERTY |
    ////////////////////////////////////////////////////////////////////////////////

    if (prop->ArrayDim > 1) {
        uint8_t* data = reinterpret_cast<uint8_t*>(obj);

        if (impl_export_text(prop, index, temp_str, data, data, obj)) {
            out << fmt::format(L"  {}[{}]={}\n", str{prop->Name}, index, temp_str.data);
        } else {
            out << fmt::format(L"  {}[{}]={}\n", str{prop->Name}, index, dflt_err_msg);
        }

        return out.str();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // | DYNAMIC ARRAY PROPERTY |
    ////////////////////////////////////////////////////////////////////////////////

    if (prop->Class->Name == FName{ClassTraits<UArrayProperty>::NAME}) {
        auto* array_prop = reinterpret_cast<UArrayProperty*>(prop);
        const auto* inner = array_prop->get_inner();
        const WrappedArray arr = get_property(array_prop, 0, reinterpret_cast<uintptr_t>(obj));
        const auto elem_size = inner->ElementSize;

        auto addr = reinterpret_cast<uintptr_t>(arr.base->data) + (index * elem_size);
        uint8_t* data = reinterpret_cast<uint8_t*>(addr);

        if (impl_export_text_item(inner, temp_str, data, data, obj)) {
            out << fmt::format(L"  {}({})={}\n", str{prop->Name}, index, temp_str.data);
        } else {
            out << fmt::format(L"  {}({})={}\n", str{prop->Name}, index, dflt_err_msg);
        }

        return out.str();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // | STANDALONE PROPERTY |
    ////////////////////////////////////////////////////////////////////////////////

    auto* data = reinterpret_cast<uint8_t*>(obj);
    if (impl_export_text(prop, 0, temp_str, data, data, obj)) {
        out << fmt::format(L"  {}={}\n", str{prop->Name}, temp_str.data);
    } else {
        out << fmt::format(L"  {}={}\n", str{prop->Name}, dflt_err_msg);
    }

    return out.str();
}

}  // namespace text_mods
