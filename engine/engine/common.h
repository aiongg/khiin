#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace khiin::engine {

inline const std::string kKhiinHome = "KHIIN_HOME";
inline const std::string CONFIG_FILE = "taikey.json";

//#ifndef _DEBUG
inline const std::string kDatabaseFilename = "khiin.db";
//#else
//inline const std::string kDatabaseFilename = "khiin_test.db";
//#endif

using string_vector = std::vector<std::string>;

enum class CursorDirection {
    L,
    R,
};

// std::visit helper
template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace khiin::engine
