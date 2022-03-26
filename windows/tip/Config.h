#pragma once

namespace khiin::proto {
class AppConfig;
} // namespace khiin::proto

namespace khiin::win32 {

enum class UiLanguage {
    English,
    HanloTai,
    LojiTai,
};

enum class UiColors {
    Light,
    Dark,
};

enum class SystemUiColors {
    Light,
    Dark,
};

enum class Hotkey {
    None,
    AltBacktick,
    Shift,
    CtrlPeriod,
    CtrlBacktick,
};

class Config {
  public:
    static void LoadFromFile(HMODULE hmodule, proto::AppConfig *config);
    static void SaveToFile(HMODULE hmodule, proto::AppConfig *config);
    static void NotifyChanged();
    static void ClearUserHistory();
    static void CycleInputMode(proto::AppConfig *config);
    static UiLanguage GetSystemLang();
    static UiColors GetUiColors();
    static void SetUiColors(UiColors colors);
    static UiLanguage GetUiLanguage();
    static void SetUiLanguage(UiLanguage lang);
    static int GetUiSize();
    static void SetUiSize(int size);
    static Hotkey GetOnOffHotkey();
    static void SetOnOffHotkey(Hotkey key);
    static Hotkey GetInputModeHotkey();
    static void SetInputModeHotkey(Hotkey key);
    static std::wstring GetDatabaseFile();
    static void SetDatabaseFile(std::wstring file_path);
    static std::wstring GetUserDictionaryFile();
    static void SetUserDictionaryFile(std::wstring file_path);
    static bool SystemUsesLightTheme();
};

namespace tip {
struct ConfigChangeListener {
    virtual void OnConfigChanged(proto::AppConfig *config) = 0;
};
} // namespace tip

} // namespace khiin::win32
