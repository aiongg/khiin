#pragma once

#include <filesystem>

#include "Utils.h"

namespace khiin::win32 {

class Files {
  public:
    static std::filesystem::path GetTempFolder();
    static std::filesystem::path GetFilePath(HMODULE hmodule, std::string_view filename);
};

} // namespace khiin::win32