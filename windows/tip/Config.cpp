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

#include "Files.h"
#include "Guids.h"
#include "Registrar.h"

namespace khiin::win32 {

namespace {
using namespace winrt;
using namespace proto;
namespace fs = std::filesystem;
using namespace khiin::win32::tip;

constexpr std::string_view kConfigFilename = "khiin_config.json";
constexpr std::string_view kIniFilename = "khiin_config.ini";

// Ini file settings
namespace ini {
constexpr const char *yes = "true";
constexpr const char *no = "false";

constexpr const char *engine = "engine";
constexpr const char *input_mode = "input_mode";
constexpr const char *continuous = "continuous";
constexpr const char *basic = "basic";
constexpr const char *manual = "manual";
constexpr const char *enabled = "enabled";
constexpr const char *dotted_khin = "dotted_khin";
constexpr const char *autokhin = "autokhin";
constexpr const char *easy_ch = "easy_ch";
constexpr const char *uppercase_nasal = "uppercase_nasal";
constexpr const char *user_dictionary = "user_dictionary";

constexpr const char *ui = "ui";
constexpr const char *theme = "theme";
constexpr const char *light = "light";
constexpr const char *dark = "dark";
constexpr const char *language = "language";
constexpr const char *hanlo = "hanlo";
constexpr const char *loji = "loji";
constexpr const char *english = "english";
constexpr const char *size = "size";

constexpr const char *shortcuts = "shortcuts";
constexpr const char *on_off = "on_off";
constexpr const char *shift = "shift";
constexpr const char *alt_backtick = "alt+backtick";
constexpr const char *ctrl_backtick = "ctrl+backtick";


} // namespace ini

// Registry settings
namespace settings {
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

UiLanguage Config::GetSystemLang() {
    if (auto lang = PRIMARYLANGID(::GetUserDefaultUILanguage());
        lang == LANG_CHINESE || lang == LANG_CHINESE_TRADITIONAL) {
        return UiLanguage::HanloTai;
    }

    return UiLanguage::English;
}

std::unique_ptr<CSimpleIniA> GetIniFile(HMODULE hmodule) {
    auto conf_file = Files::GetFilePath(hmodule, kIniFilename);
    if (fs::exists(conf_file)) {
        auto ret = std::make_unique<CSimpleIniA>();
        ret->SetUnicode();
        SI_Error rc = ret->LoadFile(conf_file.c_str());
        if (rc >= 0) {
            return ret;
        }
    }

    return nullptr;
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

    auto conf_file = Files::GetFilePath(hmodule, kIniFilename);
    auto ret = ini->SaveFile(conf_file.c_str());
    if (ret < 0) {
        auto x = 3;
    }
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

void Config::SetUserDictionaryFile(std::wstring file_path) {
    Registrar::SetSettingsString(settings::kUserDictionaryFile, file_path);
}

std::wstring Config::GetUserDictionaryFile() {
    return Registrar::GetSettingsString(settings::kUserDictionaryFile);
}

void Config::SetInputModeHotkey(Hotkey key) {
    Registrar::SetSettingsInt(settings::kInputModeHotkey, EnumInt(key));
}

bool Config::SystemUsesLightTheme() {
    return Registrar::SystemUsesLightTheme();
}

} // namespace khiin::win32
