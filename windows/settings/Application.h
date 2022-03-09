#pragma once

namespace khiin::win32::settings {

class Application {
  public:
    virtual proto::UiLanguage uilang() = 0;
    virtual void set_uilang(proto::UiLanguage lang) = 0;
};

} // namespace khiin::win32::settings
