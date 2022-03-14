#pragma once

namespace khiin::win32 {
enum class UiLanguage;
}

namespace khiin::win32::settings {

class Application {
  public:
    virtual void Reinitialize() = 0;
    virtual int ShowDialog() = 0;
};

} // namespace khiin::win32::settings
