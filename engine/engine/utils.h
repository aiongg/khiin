#pragma once

#include <cstdio>
#include <filesystem>
#include <string>

#include "common.h"

namespace khiin::engine::utils {

namespace {
/**
 * Convert all std::strings to const char* using constexpr if (C++17)
 */
template <typename T>
auto convert(T &&t) {
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
    if (size <= 0) {
        throw std::runtime_error("Error during formatting.");
    }
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1);
}
} // namespace

namespace fs = std::filesystem;

const inline std::string_view make_string_view(std::string const &str, std::string::const_iterator first,
                                               std::string::const_iterator last) noexcept {
    return std::string_view(str.data() + (first - str.begin()), last - first);
}

const inline std::vector<std::string> split_string(std::string_view str, char delimiter) {
    auto ret = std::vector<std::string>();
    auto begin = size_t(0);
    auto end = str.find(delimiter);
    while (end != std::string::npos) {
        ret.emplace_back(str.substr(begin, end - begin));
        begin = end + 1;
        end = str.find(delimiter, begin);
    }
    ret.emplace_back(str.substr(begin, end));
    return ret;
}

template <typename... Args>
std::string format(std::string fmt, Args &&...args) {
    return format_internal(fmt, convert(std::forward<Args>(args))...);
}

inline fs::path findResourceDirectory() {
#pragma warning(push)
#pragma warning(disable : 4996)
    auto tkpath = ::getenv(kKhiinHome.c_str());
#pragma warning(pop)

    if (tkpath == nullptr) {
        return fs::path();
    }

#ifdef _WIN32
    auto searchDirectories = split_string(tkpath, ';');
#else
    auto searchDirectories = split_string(tkpath, ':');
#endif
    fs::path path;
    for (auto &dir : searchDirectories) {
        path = fs::path(dir) /= kDatabaseFilename;
        if (fs::exists(path)) {
            return fs::path(dir);
        }
    }

    return fs::path();
}

} // namespace khiin::engine::utils