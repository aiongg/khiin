#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace khiin::engine {

inline const std::string kKhiinHome = "KHIIN_HOME";

using string_vector = std::vector<std::string>;

using utf8_size_t = size_t; // number of UTF codepoints, 1-4 bytes

enum class CursorDirection {
    L,
    R,
};


} // namespace khiin::engine
