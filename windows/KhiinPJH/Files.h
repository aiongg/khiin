#pragma once

#include <filesystem>

#include "Utils.h"

namespace khiin::win32 {

inline constexpr std::string_view kAppDataFolder = "Khiin PJH";

class Files {
  public:
    inline static std::filesystem::path GetFilePath(HMODULE hmodule, std::string_view filename) {
        wchar_t *tmp;
        if (::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &tmp) == S_OK) {
            auto path = std::filesystem::path(Utils::Narrow(std::wstring(tmp)));
            ::CoTaskMemFree(tmp);
            path /= kAppDataFolder;
            path /= filename;
            if (std::filesystem::exists(path)) {
                return path;
            }
        } else {
            ::CoTaskMemFree(tmp);
        }

        if (hmodule) {
            wchar_t module_path[MAX_PATH] = {};
            auto len = ::GetModuleFileName(hmodule, &module_path[0], MAX_PATH);
            auto path = std::filesystem::path(module_path);
            path.replace_filename("resources");
            path /= filename;
            return path;
        }

        return std::filesystem::path(filename);
    };
};

} // namespace khiin::win32
