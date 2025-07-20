//
// Date       : 20/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

namespace tm_parse {

using table_ref = size_t;
constexpr table_ref invalid_ref_v = std::numeric_limits<table_ref>::max();

class NameTable {
   private:
    std::deque<str> m_NameList;
    std::unordered_map<str_view, table_ref> m_NameMap;

   public:
    NameTable() = default;
    ~NameTable() = default;

   public:
    // clang-format off
    decltype(auto) begin() noexcept       { return m_NameList.begin(); }
    decltype(auto) end() noexcept         { return m_NameList.end();   }
    decltype(auto) begin() const noexcept { return m_NameList.begin(); }
    decltype(auto) end() const noexcept   { return m_NameList.end();   }

    size_t size() const noexcept                { return m_NameList.size();                       }
    bool empty() const noexcept                 { return m_NameList.empty();                      }
    bool contains(table_ref ref) const noexcept { return ref < m_NameList.size();                 }
    bool contains(str_view name) const noexcept { return m_NameMap.find(name) != m_NameMap.end(); }
    // clang-format on

   public:
    str_view find(table_ref ref) const noexcept {
        if (ref >= m_NameList.size()) {
            return str_view{};
        }
        return m_NameList[ref];
    }

    table_ref find(str_view name) const noexcept {
        auto it = m_NameMap.find(name);
        if (it == m_NameMap.end()) {
            return invalid_ref_v;
        }
        return it->second;
    }

   public:
    table_ref register_name(str_view name) noexcept {
        auto it = m_NameMap.find(name);

        // Name already exists
        if (it != m_NameMap.end()) {
            return it->second;
        }

        // Name does not exist copy into & return
        table_ref name_ref = m_NameList.size();
        const str& str = m_NameList.emplace_back(name);
        m_NameMap.emplace(str, name_ref);

        return name_ref;
    }
};

}  // namespace tm_parse