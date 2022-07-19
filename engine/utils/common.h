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
// inline const std::string kDatabaseFilename = "khiin_test.db";
//#endif

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

template <typename IterT>
size_t safe_advance(IterT &it, IterT &end, size_t n) {
    size_t i = 0;
    for (; i != n && it != end; ++i) {
        ++it;
    }
    return n - i;
}

template <typename IterT>
size_t safe_advance(IterT &it, IterT &&end, size_t n) {
    size_t i = 0;
    for (; i != n && it != end; ++i) {
        ++it;
    }
    return n - i;
}

template <typename IterT>
size_t safe_reverse(IterT &it, IterT &begin, size_t n) {
    for (; n != 0 && it != begin; --n) {
        --it;
    }
    return n;
}

template <typename IterT>
size_t safe_reverse(IterT &it, IterT &&begin, size_t n) {
    for (; n != 0 && it != begin; --n) {
        --it;
    }
    return n;
}

} // namespace khiin::engine
