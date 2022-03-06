#pragma once

#include <filesystem>

namespace khiin {

class Logger {
  public:
    static void Initialize(std::filesystem::path log_folder);
    static void Uninitialize();
};

} // namespace khiin
