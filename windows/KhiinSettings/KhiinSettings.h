#pragma once

namespace khiin::win32::settings {

enum class UiLanguage {
    NotSet,
    EN,
    HL,
    LO,
};

class KhiinSettings {
  public:
    virtual UiLanguage uilang() = 0;
    virtual void set_uilang(UiLanguage lang) = 0;
    virtual messages::AppConfig *config() = 0;
};

} // namespace khiin::win32::settings
