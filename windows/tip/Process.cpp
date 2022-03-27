#include "pch.h"

#include <filesystem>

#include "Process.h"

#include "Config.h"

namespace khiin::win32 {
namespace fs = std::filesystem;

void Process::OpenSettingsApp(HMODULE hmodule) {
    if (auto path = Config::GetSettingsAppPath(hmodule); !path.empty() && fs::exists(path)) {
        STARTUPINFO startup_info = {sizeof(startup_info)};
        PROCESS_INFORMATION process_info = {};
        if (::CreateProcess(path.c_str(), NULL, NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info)) {
            ::CloseHandle(process_info.hProcess);
            ::CloseHandle(process_info.hThread);
        }
    }
}

void Process::StartServerApp(HMODULE hmodule) {
    if (auto path = Config::GetServerAppPath(hmodule); !path.empty() && fs::exists(path)) {
        STARTUPINFO startup_info = {sizeof(startup_info)};
        PROCESS_INFORMATION process_info = {};
        if (::CreateProcess(path.c_str(), NULL, NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info)) {
            ::CloseHandle(process_info.hProcess);
            ::CloseHandle(process_info.hThread);
        }
    }
}

} // namespace khiin::win32
