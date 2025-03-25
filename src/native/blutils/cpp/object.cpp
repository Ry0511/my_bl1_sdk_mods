//
// Date       : 23/03/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "object.h"

#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/prop_traits.h"

#include "unrealsdk/format.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/utils.h"

////////////////////////////////////////////////////////////////////////////////
// | INTERNAL |
////////////////////////////////////////////////////////////////////////////////

namespace {

using namespace blutils;

// TODO: Not sure if more than one property time has issues with pointers
UClass* ubool_prop_cls = find_class(L"Core.BoolProperty"_fn);

void* get_property_ptr(UProperty* prop, UObject* obj, size_t index = 0) {
    auto base_addr = reinterpret_cast<uintptr_t>(obj);
    return reinterpret_cast<void*>(base_addr + prop->Offset_Internal + (index * prop->ElementSize));
}

template <class T>
PropTraits<T>::Value get_property_value(UProperty* prop, UObject* obj, size_t index = 0) {
    return get_property<T>(reinterpret_cast<T*>(prop), index, reinterpret_cast<uintptr_t>(obj));
};

template <class T, class ValueType = PropTraits<T>::Value>
void set_property_value(UProperty* prop, UObject* obj, ValueType value, size_t index = 0) {
    set_property<T>(reinterpret_cast<T*>(prop), index, reinterpret_cast<uintptr_t>(obj), value);
}

// These were discovered alongside ImportText when tracing BL1's set command
constexpr auto INDEX_PRE_EDIT_CHANGE = 0x11;
constexpr auto INDEX_POST_EDIT_CHANGE = 0x13;
typedef void(__fastcall* uobj_uprop_changed_event)(UObject*, void*, UProperty*);

void _uproperty_prop_changed_event(UObject* obj, UProperty* prop, size_t index) {
    auto fn = reinterpret_cast<uobj_uprop_changed_event>(obj->vftable[index]);
    fn(obj, nullptr, prop);
}

bool _uproperty_import_text(UProperty* prop, const wchar_t* text, uint32_t flags, UObject* parent) {
    // Get import text function
    constexpr auto INDEX_IMPORT_TEXT = 0x54;
    typedef void*(__fastcall * uprop_import_text)(UProperty*, void*, const wchar_t*, void*, uint32_t, UObject*, void*);
    auto fn_import_text = reinterpret_cast<uprop_import_text>(prop->vftable[INDEX_IMPORT_TEXT]);

    void* result = nullptr;

    _uproperty_prop_changed_event(parent, prop, INDEX_PRE_EDIT_CHANGE);
    if (prop->Class == ubool_prop_cls) {
        bool val = get_property_value<UBoolProperty>(prop, parent);
        result = fn_import_text(
            prop,
            nullptr,
            text,
            reinterpret_cast<void*>(&val),
            flags,
            parent,
            nullptr
        );

    } else {
        void* p = get_property_ptr(prop, parent);
        result = fn_import_text(prop, nullptr, text, p, flags, parent, nullptr);
    }
    _uproperty_prop_changed_event(parent, prop, INDEX_POST_EDIT_CHANGE);

    return result != nullptr;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// | EXTERNAL |
////////////////////////////////////////////////////////////////////////////////

namespace blutils {

bool import_text(
    UObject* obj,
    UProperty* prop,
    const std::wstring& text,
    uint32_t import_flags,
    uint64_t obj_flags
) {
    bool ok = _uproperty_import_text(prop, text.c_str(), import_flags, obj);
    if (ok) {
        obj->ObjectFlags |= obj_flags;
    }
    return ok;
}

std::optional<UObject*> find_object(const std::wstring& obj_path_name, bool bthrow) {
    for (UObject* obj : gobjects()) {
        if (!obj) {
            continue;
        }

        std::wstring pname = obj->get_path_name();
        if (pname == obj_path_name) {
            return std::make_optional(obj);
        }
    }

    if (bthrow) {
        throw py::value_error{format("Could not find object: '{}'", utils::narrow(obj_path_name))};
    }

    return std::nullopt;
}

}  // namespace blutils
