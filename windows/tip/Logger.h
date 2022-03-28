#pragma once

#include <filesystem>

namespace khiin::win32 {

class Logger {
  public:
    static void Initialize(std::filesystem::path log_folder);
    static void Uninitialize();
};

} // namespace khiin
