#pragma once

#include <bitset>
#include <cstdio>
#include <string>
#include <filesystem>

#include "common.h"

namespace khiin::engine::utils {

namespace {
/**
 * Convert all std::strings to const char* using constexpr if (C++17)
 */
template <typename T>
auto to_const_char_ptr(T &&t) {
    if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value) {
        return std::forward<T>(t).c_str();
    } else {
        return std::forward<T>(t);
    }
}

/**
 * printf like formatting for C++ with std::string
 * Original source: https://stackoverflow.com/a/26221725/11722
 */
template <typename... Args>
std::string format_internal(const std::string &format, Args &&...args) {
    size_t size = snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...) + 1;
    if (size == 0) {
        throw std::runtime_error("Error during formatting.");
    }
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1);
}

} // namespace

template <typename... Args>
std::string format(std::string fmt, Args &&...args) {
    return format_internal(fmt, to_const_char_ptr(std::forward<Args>(args))...);
}

const inline std::string_view make_string_view(std::string const &str, std::string::const_iterator first,
                                               std::string::const_iterator last) noexcept {
    return std::string_view(str.data() + (first - str.begin()), last - first);
}

std::vector<std::string> split_string(std::string_view str, char delimiter);
std::vector<std::string> split_string_multi(std::string_view str, std::string_view delimiters);

std::filesystem::path findResourceDirectory();

inline int bitscan_forward(uint64_t x) {
    auto bits = std::bitset<64>((x & -static_cast<int64_t>(x)) - 1);
    return static_cast<int>(bits.count());
}

inline int bitscan_reverse(uint64_t x) {
    int ret = 0;
    while (x >>= 1) {
        ++ret;
    }
    return ret;
}

inline std::vector<int> bitpositions(uint64_t bb_ull) {
    auto ret = std::vector<int>();
    while (bb_ull != 0) {
        auto pos = bitscan_forward(bb_ull);
        auto bits = std::bitset<64>(bb_ull);
        bits.set(pos, false);
        bb_ull = bits.to_ullong();
        ret.push_back(pos);
    }
    return ret;
}

} // namespace khiin::engine::utils
