//
// Date       : 23/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//
#ifndef TEXT_MODS_PARSER_PCH_H
#define TEXT_MODS_PARSER_PCH_H

#if defined(TEXT_MODS_UNREALSDK)
#include "unrealsdk/pch.h"
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <charconv>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace tm_parse {
namespace fs = std::filesystem; // The 'I aint writing all that' alias
}

#endif
