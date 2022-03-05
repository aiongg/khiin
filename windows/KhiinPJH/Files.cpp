#include "pch.h"

#include "Files.h"

namespace khiin::win32 {
namespace {

namespace fs = std::filesystem;
inline constexpr std::string_view kAppDataFolder = "Khiin PJH";
inline constexpr std::string_view kModuleFolderDataFolder = "resources";

}

fs::path Files::GetTempFolder() {
    wchar_t tmp[MAX_PATH];
    ::GetTempPath(MAX_PATH, tmp);
    auto path = fs::path(Utils::Narrow(std::wstring(tmp)));
    return path;
}

fs::path Files::GetFilePath(HMODULE hmodule, std::string_view filename) {
    wchar_t *tmp;
    if (::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &tmp) == S_OK) {
        auto path = fs::path(Utils::Narrow(std::wstring(tmp)));
        ::CoTaskMemFree(tmp);
        path /= kAppDataFolder;
        path /= filename;
        if (fs::exists(path)) {
            return path;
        }
    } else {
        ::CoTaskMemFree(tmp);
    }

    if (hmodule) {
        wchar_t module_path[MAX_PATH] = {};
        auto len = ::GetModuleFileName(hmodule, &module_path[0], MAX_PATH);
        auto path = fs::path(module_path);
        path.replace_filename(kModuleFolderDataFolder);
        path /= filename;
        return path;
    }

    return fs::path(filename);
};

} // namespace khiin::win32