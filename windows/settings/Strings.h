#pragma once

namespace khiin::win32::settings {

class Strings {
  public:
    static std::wstring T(uint32_t rid, proto::UiLanguage lang);
};

} // namespace khiin::win32::settings
