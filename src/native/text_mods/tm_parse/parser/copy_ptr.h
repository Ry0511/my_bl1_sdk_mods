//
// Date       : 22/06/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

namespace tm_parse {

//
// More of a ClonePtr really, lets you copy the underlying data without sharing/transferring ownership
// effectively creating a new distinct object which is owned by the CopyPtr. Move semantics are the
// same as with a regular unique pointer, that is, ownership is transferred and the original is
// invalidated.
//
template <class T>
class CopyPtr {
   private:
    std::unique_ptr<T> m_Ptr;

   public: // clang-format off
    constexpr CopyPtr() noexcept = default;                    // null ctor
    constexpr CopyPtr(nullptr_t) noexcept : m_Ptr(nullptr) {}; // implicit nullptr to CopyPtr
    CopyPtr(const T& val) noexcept : m_Ptr(std::make_unique<T>(val)) {}
    // clang-format on

    ~CopyPtr() = default;

   public:
    CopyPtr(const CopyPtr& other) : m_Ptr((other == nullptr) ? nullptr : std::make_unique<T>(*other)) {}
    CopyPtr& operator=(const CopyPtr& other) {
        m_Ptr = (other == nullptr) ? nullptr : std::make_unique<T>(*other);
        return *this;
    }

    CopyPtr(CopyPtr&&) = default;
    CopyPtr& operator=(CopyPtr&&) = default;

    public:
    bool operator==(const CopyPtr<T>& other) const noexcept = default;
    bool operator!=(const CopyPtr<T>& other) const noexcept = default;

   public: // clang-format off
    const T* get() const { return m_Ptr.get(); }
    T*       get()       { return m_Ptr.get(); }

   public:
    const T* operator->() const { return get(); }
    T*       operator->()       { return get(); };

    const T& operator*() const { return *m_Ptr; };
    T&       operator*()       { return *m_Ptr; };

    // clang-format on
};

}  // namespace tm_parse
