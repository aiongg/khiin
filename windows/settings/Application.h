#pragma once

namespace khiin::win32::settings {

class Application {
  public:
    virtual messages::UiLanguage uilang() = 0;
    virtual void set_uilang(messages::UiLanguage lang) = 0;
};

} // namespace khiin::win32::settings
