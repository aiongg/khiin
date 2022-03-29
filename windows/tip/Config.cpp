#include "pch.h"

#include "Config.h"

#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <sstream>

#define SI_NO_CONVERSION

#include "simpleini/SimpleIni.h"

#include "proto/proto.h"

#include "Guids.h"
#include "Registrar.h"

namespace khiin::win32 {

namespace {
using namespace winrt;
using namespace proto;
namespace fs = std::filesystem;
using namespace khiin::win32::tip;

const std::wstring kAppDataFolder = L"Khiin PJH";
const std::wstring kModuleDirDataFolder = L"resources";
const std::wstring kModuleDirFontFolder = L"fonts";
const std::wstring kIniFilename = L"khiin_config.ini";
const std::wstring kDatabaseFilename = L"khiin.db";
const std::wstring kSettingsAppFilename = L"KhiinSettings.exe";
const std::wstring kUserDbFilename = L"khiin_userdb.txt";
const std::vector<std::wstring> kFontFiles = {L"SourceHanSansTW-Normal.otf"};

// Ini file settings
namespace ini {
constexpr auto yes = "true";
constexpr auto no = "false";

constexpr auto engine = "engine";
constexpr auto input_mode = "input_mode";
constexpr auto continuous = "continuous";
constexpr auto basic = "basic";
constexpr auto manual = "manual";
constexpr auto enabled = "enabled";
constexpr auto dotted_khin = "dotted_khin";
constexpr auto autokhin = "autokhin";
constexpr auto easy_ch = "easy_ch";
constexpr auto uppercase_nasal = "uppercase_nasal";
constexpr auto user_dictionary = "user_dictionary";

constexpr auto ui = "ui";
constexpr auto theme = "theme";
constexpr auto light = "light";
constexpr auto dark = "dark";
constexpr auto language = "language";
constexpr auto hanlo = "hanlo";
constexpr auto loji = "loji";
constexpr auto english = "english";
constexpr auto size = "size";

constexpr auto shortcuts = "shortcuts";
constexpr auto on_off = "on_off";
constexpr auto shift = "shift";
constexpr auto alt_backtick = "alt+backtick";
constexpr auto ctrl_backtick = "ctrl+backtick";

} // namespace ini

// Registry settings
namespace settings {
const std::wstring settings_app = L"settings_exe";
const std::wstring database_file = L"database";
const std::wstring config_file = L"config_ini";
const std::wstring user_db_file = L"user_db";
const std::wstring font_dir = L"font_dir";

const std::wstring kUiColors = L"UiColors";
const std::wstring kUiSize = L"UiSize";
const std::wstring kUiLanguage = L"UiLanguage";
const std::wstring kOnOffHotkey = L"OnOffHotkey";
const std::wstring kInputModeHotkey = L"InputModeHotkey";
const std::wstring kUserDictionaryFile = L"UserDictionaryFile";
} // namespace settings

bool eq(const char *lhs, const char *rhs) {
    return std::strcmp(lhs, rhs) == 0;
}

template <typename EnumT>
int EnumInt(EnumT e) {
    return static_cast<int>(e);
}

template <typename EnumT>
EnumT EnumVal(std::underlying_type_t<EnumT> e) {
    return static_cast<EnumT>(e);
}

const std::wstring DefaultFilename(KhiinFile file) {
    switch (file) {
    case KhiinFile::Config:
        return kIniFilename;
    case KhiinFile::Database:
        return kDatabaseFilename;
    case KhiinFile::SettingsApp:
        return kSettingsAppFilename;
    case KhiinFile::UserDb:
        return kUserDbFilename;
    }

    // should never reach here
    assert("Invalid KhiinFile");
    return std::wstring();
}

const std::wstring FilenameRegistryKey(KhiinFile file) {
    switch (file) {
    case KhiinFile::Config:
        return settings::config_file;
    case KhiinFile::Database:
        return settings::database_file;
    case KhiinFile::SettingsApp:
        return settings::settings_app;
    case KhiinFile::UserDb:
        return settings::user_db_file;
    }

    // should never reach here
    assert("Invalid KhiinFile");
    return std::wstring();
}

const void LoadModuleHandle(HMODULE &hmodule) {
    if (hmodule != NULL) {
        return;
    }

    if (::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<wchar_t *>(LoadModuleHandle), &hmodule) != 0) {
        return;
    }

    hmodule = NULL;
}

const fs::path GetRoamingAppDataDir() {
    auto ret = fs::path();

    wchar_t *tmp;
    if (::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &tmp) == S_OK) {
        auto path = fs::path(tmp);
        ::CoTaskMemFree(tmp);
        path /= kAppDataFolder;

        if (fs::exists(path)) {
            ret = path;
        }
    } else {
        ::CoTaskMemFree(tmp);
    }

    return ret;
}

const fs::path GetModuleDir(HMODULE hmodule) {
    LoadModuleHandle(hmodule);

    if (hmodule == NULL) {
        return fs::path();
    }

    wchar_t tmp[MAX_PATH] = {};
    auto len = ::GetModuleFileName(hmodule, &tmp[0], MAX_PATH);
    auto path = fs::path(tmp);
    path.remove_filename();

    if (fs::exists(path)) {
        return path;
    }

    return fs::path();
}

const std::wstring FindFileInRoamingAppData(std::wstring_view filename) {
    if (auto path = GetRoamingAppDataDir(); !path.empty()) {
        path /= filename;
        if (fs::exists(path)) {
            return path.wstring();
        }
    }

    return std::wstring();
}

const std::wstring FindFileInModulePath(HMODULE hmodule, std::wstring_view filename) {
    if (auto path = GetModuleDir(hmodule); !path.empty()) {
        fs::path res_path = path;
        res_path /= kModuleDirDataFolder;
        res_path /= filename;

        if (fs::exists(res_path)) {
            return res_path.wstring();
        }

        path /= filename;

        if (fs::exists(path)) {
            return path.wstring();
        }
    }

    return std::wstring();
}

const std::wstring FindFontDir(HMODULE hmodule) {
    if (auto path = GetModuleDir(hmodule); !path.empty()) {
        path /= kModuleDirFontFolder;

        if (fs::exists(path)) {
            return path.wstring();
        }
    }

    return std::wstring();
}

DWORD Timestamp() {
#pragma warning(push)
#pragma warning(disable : 28159)
    return ::GetTickCount();
#pragma warning(pop)
}

void NotifyGlobalCompartment(GUID compartment_guid, DWORD value) {
    auto thread_mgr = com_ptr<ITfThreadMgr>();
    TfClientId client_id = 0;
    auto compartment_mgr = com_ptr<ITfCompartmentMgr>();
    auto compartment = com_ptr<ITfCompartment>();
    VARIANT var;
    ::VariantInit(&var);
    var.vt = VT_I4;
    var.lVal = value;

    check_hresult(::CoInitialize(NULL));
    check_hresult(
        ::CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, thread_mgr.put_void()));
    check_hresult(thread_mgr->Activate(&client_id));
    check_hresult(thread_mgr->GetGlobalCompartment(compartment_mgr.put()));
    check_hresult(compartment_mgr->GetCompartment(compartment_guid, compartment.put()));
    check_hresult(compartment->SetValue(client_id, &var));
}

InputMode NextInputMode(InputMode current_mode) {
    switch (current_mode) {
    case IM_CONTINUOUS:
        return IM_BASIC;
    case IM_BASIC:
        return IM_PRO;
    case IM_PRO:
        return IM_CONTINUOUS;
    default:
        return current_mode;
    }
}

} // namespace

std::wstring Config::GetKnownFile(KhiinFile file, HMODULE hmodule, std::wstring const &file_path_override) {
    auto ret = Registrar::GetSettingsString(FilenameRegistryKey(file));

    if (!ret.empty() && fs::exists(ret)) {
        return ret;
    }

    const auto filename = file_path_override.empty() ? DefaultFilename(file) : file_path_override;

    ret = FindFileInRoamingAppData(filename);
    if (!ret.empty() && fs::exists(ret)) {
        return ret;
    }

    ret = FindFileInModulePath(hmodule, filename);
    if (!ret.empty() && fs::exists(ret)) {
        return ret;
    }

    return std::wstring();
}

void Config::SetKnownFilePath(KhiinFile file, std::wstring const &file_path) {
    if (!file_path.empty() && fs::exists(file_path)) {
        Registrar::SetSettingsString(FilenameRegistryKey(file), file_path);
    }
}

const inline bool IsFontFile(fs::path const &path) {
    return path.has_extension() && (path.extension() == ".otf" || path.extension() == ".ttf");
}

std::vector<std::wstring> Config::GetFontFiles() {
    auto ret = std::vector<std::wstring>();

    auto font_dir = Registrar::GetSettingsString(settings::font_dir);

    if (font_dir.empty()) {
        font_dir = FindFontDir(NULL);
    }

    for (const auto &file : fs::directory_iterator(font_dir)) {
        if (file.is_regular_file()) {
            auto const &path = file.path();

            if (IsFontFile(path)) {
                ret.push_back(path.wstring());
            }
        }
    }

    return ret;
}

UiLanguage Config::GetSystemLang() {
    if (auto lang = PRIMARYLANGID(::GetUserDefaultUILanguage());
        lang == LANG_CHINESE || lang == LANG_CHINESE_TRADITIONAL) {
        return UiLanguage::HanloTai;
    }

    return UiLanguage::English;
}

std::unique_ptr<CSimpleIniA> GetIniFile(HMODULE hmodule) {
    auto ret = std::make_unique<CSimpleIniA>();
    auto conf_file = Config::GetKnownFile(KhiinFile::Config, hmodule);

    if (fs::exists(conf_file)) {
        ret->SetUnicode();
        SI_Error rc = ret->LoadFile(conf_file.c_str());
        if (rc >= 0) {
            return ret;
        }
    }

    return ret;
}

void Config::LoadFromFile(HMODULE hmodule, AppConfig *config) {
    auto ini = GetIniFile(hmodule);
    const char *value;

    {
        value = ini->GetValue(ini::engine, ini::input_mode, ini::continuous);
        InputMode mode = IM_CONTINUOUS;
        if (eq(value, ini::basic)) {
            mode = IM_BASIC;
        } else if (eq(value, ini::manual)) {
            mode = IM_PRO;
        }
        config->set_input_mode(mode);
    }

    {
        value = ini->GetValue(ini::engine, ini::enabled, ini::yes);
        config->mutable_ime_enabled()->set_value(eq(value, ini::yes));
    }

    {
        value = ini->GetValue(ini::engine, ini::dotted_khin, ini::yes);
        config->mutable_dotted_khin()->set_value(eq(value, ini::yes));
    }

    {
        value = ini->GetValue(ini::engine, ini::autokhin, ini::yes);
        config->mutable_autokhin()->set_value(eq(value, ini::yes));
    }

    {
        value = ini->GetValue(ini::engine, ini::easy_ch, ini::no);
        config->mutable_easy_ch()->set_value(eq(value, ini::yes));
    }

    {
        value = ini->GetValue(ini::engine, ini::uppercase_nasal, ini::yes);
        config->mutable_uppercase_nasal()->set_value(eq(value, ini::yes));
    }
}

void Config::SaveToFile(HMODULE hmodule, AppConfig *config) {
    auto ini = GetIniFile(hmodule);
    const char *value;

    {
        switch (config->input_mode()) {
        case IM_BASIC:
            value = ini::basic;
            break;
        case IM_PRO:
            value = ini::manual;
            break;
        default:
            value = ini::continuous;
            break;
        }

        ini->SetValue(ini::engine, ini::input_mode, value);
    }

    {
        value = config->mutable_ime_enabled()->value() ? ini::yes : ini::no;
        ini->SetValue(ini::engine, ini::enabled, value);
    }

    {
        value = config->mutable_dotted_khin()->value() ? ini::yes : ini::no;
        ini->SetValue(ini::engine, ini::dotted_khin, value);
    }

    {
        value = config->mutable_autokhin()->value() ? ini::yes : ini::no;
        ini->SetValue(ini::engine, ini::autokhin, value);
    }

    {
        value = config->mutable_easy_ch()->value() ? ini::yes : ini::no;
        ini->SetValue(ini::engine, ini::easy_ch, value);
    }

    {
        value = config->mutable_uppercase_nasal()->value() ? ini::yes : ini::no;
        ini->SetValue(ini::engine, ini::uppercase_nasal, value);
    }

    auto conf_file = Config::GetKnownFile(KhiinFile::Config, hmodule);
    auto ret = ini->SaveFile(conf_file.c_str());
}

void Config::NotifyChanged() {
    NotifyGlobalCompartment(guids::kConfigChangedCompartment, Timestamp());
}

void Config::ClearUserHistory() {
    NotifyGlobalCompartment(guids::kResetUserdataCompartment, Timestamp());
}

void Config::CycleInputMode(proto::AppConfig *config) {
    if (!config->ime_enabled().value()) {
        config->mutable_ime_enabled()->set_value(true);
    } else {
        config->set_input_mode(NextInputMode(config->input_mode()));
    }
}

UiColors Config::GetUiColors() {
    return EnumVal<UiColors>(Registrar::GetSettingsInt(settings::kUiColors));
}

void Config::SetUiColors(UiColors colors) {
    Registrar::SetSettingsInt(settings::kUiColors, EnumInt(colors));
}

UiLanguage Config::GetUiLanguage() {
    return EnumVal<UiLanguage>(Registrar::GetSettingsInt(settings::kUiLanguage));
}

void Config::SetUiLanguage(UiLanguage lang) {
    Registrar::SetSettingsInt(settings::kUiLanguage, EnumInt(lang));
}

int Config::GetUiSize() {
    return Registrar::GetSettingsInt(settings::kUiSize);
}

void Config::SetUiSize(int size) {
    Registrar::SetSettingsInt(settings::kUiSize, size);
}

Hotkey Config::GetOnOffHotkey() {
    auto i = Registrar::GetSettingsInt(settings::kOnOffHotkey);
    return i > 0 ? EnumVal<Hotkey>(i) : Hotkey::Shift;
}

void Config::SetOnOffHotkey(Hotkey key) {
    Registrar::SetSettingsInt(settings::kOnOffHotkey, EnumInt(key));
}

Hotkey Config::GetInputModeHotkey() {
    auto i = Registrar::GetSettingsInt(settings::kInputModeHotkey);
    return i > 0 ? EnumVal<Hotkey>(i) : Hotkey::CtrlBacktick;
}

void Config::SetInputModeHotkey(Hotkey key) {
    Registrar::SetSettingsInt(settings::kInputModeHotkey, EnumInt(key));
}

bool Config::SystemUsesLightTheme() {
    return Registrar::SystemUsesLightTheme();
}

} // namespace khiin::win32
