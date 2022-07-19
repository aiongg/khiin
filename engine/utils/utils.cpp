#include "utils/utils.h"


namespace khiin::engine::utils {

namespace fs = std::filesystem;

fs::path findResourceDirectory() {
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

std::vector<std::string> split_string(std::string_view str, char delimiter) {
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

std::vector<std::string> split_string_multi(std::string_view str, std::string_view delimiters) {
    auto ret = std::vector<std::string>();

    //auto start = str.cbegin();
    auto end = str.cbegin();

    while (end != str.cend()) {
        for (auto delim = delimiters.cbegin(); delim != delimiters.cend(); ++delim) {
            if (*end == *delim) {
                //start = end;
                ++end;
                break;
            }
        }
        ++end;
    }
    return ret;
}

} // namespace khiin::engine::utils