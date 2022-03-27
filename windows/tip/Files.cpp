#include "pch.h"

#include "Files.h"
#include "Registrar.h"

namespace khiin::win32 {
namespace {
namespace fs = std::filesystem;
using namespace winrt;

constexpr std::string_view kAppDataFolder = "Khiin PJH";
constexpr std::string_view kModuleFolderDataFolder = "resources";

std::wstring FindFileInRoamingAppData(std::wstring_view filename) {
    wchar_t *tmp;
    if (::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &tmp) == S_OK) {
        auto path = fs::path(tmp);
        ::CoTaskMemFree(tmp);
        path /= kAppDataFolder;
        path /= filename;
        if (fs::exists(path)) {
            return path.wstring();
        }
    } else {
        ::CoTaskMemFree(tmp);
    }

    return std::wstring();
}

fs::path ModulePath(HMODULE hmodule) {
    wchar_t module_path[MAX_PATH] = {};
    auto len = ::GetModuleFileName(hmodule, &module_path[0], MAX_PATH);
    auto path = fs::path(module_path);
    return path;
}

} // namespace

fs::path Files::GetTempFolder() {
    wchar_t tmp[MAX_PATH];
    ::GetTempPath(MAX_PATH, tmp);
    auto path = fs::path(Utils::Narrow(std::wstring(tmp)));
    return path;
}

std::wstring Files::GetFilePath(HMODULE hmodule, std::wstring_view filename) {
    if (auto path = FindFileInRoamingAppData(filename); !path.empty()) {
        return path;
    }
    
    if (hmodule) {
        auto mod_path = ModulePath(hmodule);
        fs::path res_path = mod_path;

        res_path.replace_filename(kModuleFolderDataFolder);
        res_path /= filename;

        if (fs::exists(res_path)) {
            return res_path.wstring();
        }

        mod_path /= filename;

        if (fs::exists(mod_path)) {
            return mod_path.wstring();
        }
    }

    if (fs::exists(filename)) {
        return std::wstring(filename);
    }

    return std::wstring();
};

} // namespace khiin::win32