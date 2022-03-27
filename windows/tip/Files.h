#pragma once

#include <filesystem>

#include "Utils.h"

namespace khiin::win32 {

class Files {
  public:
    static std::filesystem::path GetTempFolder();
    static std::wstring GetFilePath(HMODULE hmodule, std::wstring_view filename);
};

} // namespace khiin::win32
