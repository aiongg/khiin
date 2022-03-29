#pragma once

namespace khiin::proto {
class AppConfig;
} // namespace khiin::proto

namespace khiin::win32 {

enum class KhiinFile {
    Database,
    Config,
    SettingsApp,
    UserDb,
};

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
    static std::wstring GetKnownFile(KhiinFile file, HMODULE hmodule = NULL,
                                     std::wstring const &filename_override = L"");
    static void SetKnownFilePath(KhiinFile file, std::wstring const &file_path);
    static std::vector<std::wstring> GetFontFiles();
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
    static bool SystemUsesLightTheme();
};

namespace tip {
struct ConfigChangeListener {
    virtual void OnConfigChanged(proto::AppConfig *config) = 0;
};
} // namespace tip

} // namespace khiin::win32
