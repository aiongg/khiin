#include "pch.h"

#include "Files.h"
#include "common.h"

namespace khiin::win32 {
namespace {
namespace fs = std::filesystem;
using namespace winrt;
inline constexpr std::string_view kAppDataFolder = "Khiin PJH";
inline constexpr std::string_view kModuleFolderDataFolder = "resources";
inline constexpr std::wstring_view kRegistryPath = L"Software\\Khiin PJH";
inline constexpr std::wstring_view kRegistrySettingsExeValueName = L"KhiinSettings.exe";
inline constexpr std::wstring_view kDefaultSettingsExeFileName = L"KhiinSettings.exe";
} // namespace

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

fs::path Files::GetSettingsAppPath(HMODULE hmodule) {
    try {
        auto key = registry_key();
        check_win32(::RegOpenKeyEx(HKEY_CURRENT_USER, kRegistryPath.data(), 0, KEY_READ, key.put()));
        wchar_t tmp[MAX_PATH] = {};
        DWORD tmpsize = MAX_PATH;
        check_win32(::RegQueryValueEx(key.get(), kRegistrySettingsExeValueName.data(), 0, NULL, (LPBYTE)tmp, &tmpsize));
        auto path = fs::path(tmp);
        if (fs::exists(path)) {
            return path;
        }
    } catch (...) {
        // empty
    }

    if (hmodule) {
        wchar_t module_path[MAX_PATH] = {};
        auto len = ::GetModuleFileName(hmodule, &module_path[0], MAX_PATH);
        auto file = fs::path(module_path);
        file.replace_filename(kDefaultSettingsExeFileName);
        if (fs::exists(file)) {
            return file;
        }
    } else {
        auto file = fs::path(kDefaultSettingsExeFileName);
        if (fs::exists(file)) {
            return file;
        }
    }


    return fs::path();
}

} // namespace khiin::win32