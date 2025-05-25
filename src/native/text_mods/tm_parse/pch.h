//
// Date       : 23/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

////////////////////////////////////////////////////////////////////////////////
// | UNREALSDK |
////////////////////////////////////////////////////////////////////////////////

#if defined(TEXT_MODS_UNREALSDK)
#include "unrealsdk/pch.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// | STANDARD LIBRARY |
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// | INTERNAL |
////////////////////////////////////////////////////////////////////////////////

// These don't often change with the exception being tokens which is still rare
#include "common/text_mod_common.h"
#include "common/text_mod_errors.h"
#include "common/text_mod_tokens.h"
#include "common/text_mod_utils.h"

namespace tm_parse {
namespace fs = std::filesystem;  // The 'I aint writing all that' alias
}
