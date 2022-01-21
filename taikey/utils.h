#pragma once

namespace taikey::Utils {

namespace fs = std::filesystem;

auto splitString(std::string_view str, char delimiter) {
    auto ret = VStr();
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

auto findResourceDirectory() {
#pragma warning(push)
#pragma warning(disable : 4996)
    auto tkpath = ::getenv(TaiKeyPath.c_str());
#pragma warning(pop)

    if (tkpath == nullptr) {
        return fs::path();
    }

#ifdef _WIN32
    auto searchDirectories = splitString(tkpath, ';');
#else
    auto searchDirectories = splitString(tkpath, ':');
#endif
    fs::path path;
    for (auto &dir : searchDirectories) {
        path = fs::path(dir) /= DB_FILE;
        if (fs::exists(path)) {
            return fs::path(dir);
        }
    }

    return fs::path();
}

} // namespace taikey::Utils