#pragma once

namespace khiin::proto {
class AppConfig;
enum UiLanguage : int;
}

namespace khiin::win32 {

class Config {
  public:
    static void LoadFromFile(HMODULE hmodule, proto::AppConfig *config);
    static void SaveToFile(HMODULE hmodule, proto::AppConfig *config);
    static void NotifyChanged();
    static proto::UiLanguage GetSystemLang();
};

namespace tip {
struct ConfigChangeListener {
    virtual void OnConfigChanged(proto::AppConfig *config) = 0;
};
} // namespace tip

} // namespace khiin::win32
