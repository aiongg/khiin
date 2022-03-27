#pragma once

#include <filesystem>

namespace khiin {

class Logger {
  public:
    static void Initialize();
    static void Uninitialize();
};

} // namespace khiin
