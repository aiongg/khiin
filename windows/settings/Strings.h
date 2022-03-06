#pragma once

namespace khiin::win32::settings {

enum class UiLanguage;

class Strings {
  public:
    static std::wstring T(uint32_t rid, UiLanguage lang);
};

} // namespace khiin::win32::settings
