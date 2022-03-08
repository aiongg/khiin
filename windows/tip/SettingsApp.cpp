#include "pch.h"

#include "SettingsApp.h"

#include "Files.h"

namespace khiin::win32 {
namespace fs = std::filesystem;

void SettingsApp::Launch(HMODULE hmodule) {
    if (auto path = Files::GetSettingsAppPath(hmodule); fs::exists(path)) {
        STARTUPINFO startup_info = {sizeof(startup_info)};
        PROCESS_INFORMATION process_info = {};
        if (::CreateProcess(path.c_str(), NULL, NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info)) {
            ::CloseHandle(process_info.hProcess);
            ::CloseHandle(process_info.hThread);
        }
    }
}

} // namespace khiin::win32
