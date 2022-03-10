#pragma once

namespace khiin::win32 {
enum class UiLanguage;
}

namespace khiin::win32::settings {

class Strings {
  public:
    static std::wstring T(uint32_t rid, UiLanguage lang);
};

} // namespace khiin::win32::settings
