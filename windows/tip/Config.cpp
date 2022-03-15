#include "pch.h"

#include "Config.h"

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>

#include <google/protobuf/util/json_util.h>

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

const std::wstring kSettingUiColors = L"UiColors";
const std::wstring kSettingUiSize = L"UiSize";
const std::wstring kSettingUiLanguage = L"UiLanguage";
const std::wstring kSettingOnOffHotkey = L"OnOffHotkey";
const std::wstring kSettingInputModeHotkey = L"InputModeHotkey";

template <typename EnumT>
int EnumInt(EnumT e) {
    return static_cast<int>(e);
}

template <typename EnumT>
EnumT EnumVal(std::underlying_type_t<EnumT> e) {
    return static_cast<EnumT>(e);
}

} // namespace

UiLanguage Config::GetSystemLang() {
    if (auto lang = PRIMARYLANGID(::GetUserDefaultUILanguage());
        lang == LANG_CHINESE || lang == LANG_CHINESE_TRADITIONAL) {
        return UiLanguage::HanloTai;
    } else {
        return UiLanguage::English;
    }
}

void Config::LoadFromFile(HMODULE hmodule, AppConfig *config) {
    auto conf_file = Files::GetFilePath(hmodule, kConfigFilename);
    if (fs::exists(conf_file)) {
        auto f = std::ifstream(conf_file.string());
        std::stringstream buf;
        buf << f.rdbuf();
        auto json_str = buf.str();
        google::protobuf::util::JsonStringToMessage(json_str, config);
        f.close();
    }

    // if (config->appearance().ui_language() == UIL_UNSPECIFIED) {
    //    config->mutable_appearance()->set_ui_language(Config::GetSystemLang());
    //}

    if (!config->has_ime_enabled()) {
        config->mutable_ime_enabled()->set_value(true);
    }

    if (config->input_mode() == IM_UNSPECIFIED) {
        config->set_input_mode(IM_CONTINUOUS);
    }

    if (!config->has_dotted_khin()) {
        config->mutable_dotted_khin()->set_value(true);
    }

    if (!config->has_telex_enabled()) {
        config->mutable_telex_enabled()->set_value(false);
    }

    if (!config->has_autokhin()) {
        config->mutable_autokhin()->set_value(true);
    }

    if (!config->has_easy_ch()) {
        config->mutable_easy_ch()->set_value(false);
    }

    if (!config->has_uppercase_nasal()) {
        config->mutable_uppercase_nasal()->set_value(true);
    }
}

void Config::SaveToFile(HMODULE hmodule, AppConfig *config) {
    auto conf_file = Files::GetFilePath(hmodule, kConfigFilename);
    auto options = google::protobuf::util::JsonPrintOptions();
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    auto json_str = std::string();
    google::protobuf::util::MessageToJsonString(*config, &json_str, options);

    auto f = std::ofstream();
    f.open(conf_file.c_str(), std::ios::out);
    f << json_str;
    f.close();
}

void Config::NotifyChanged() {
    check_hresult(::CoInitialize(NULL));

    auto thread_mgr = com_ptr<ITfThreadMgr>();
    check_hresult(
        ::CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, thread_mgr.put_void()));

    TfClientId client_id = 0;
    check_hresult(thread_mgr->Activate(&client_id));

    auto compartment_mgr = com_ptr<ITfCompartmentMgr>();
    check_hresult(thread_mgr->GetGlobalCompartment(compartment_mgr.put()));

    auto compartment = com_ptr<ITfCompartment>();
    check_hresult(compartment_mgr->GetCompartment(guids::kConfigChangedCompartment, compartment.put()));

    VARIANT var;
    ::VariantInit(&var);
    var.vt = VT_I4;
#pragma warning(push)
#pragma warning(disable : 28159)
    var.lVal = ::GetTickCount();
#pragma warning(pop)
    check_hresult(compartment->SetValue(client_id, &var));
}

UiColors Config::GetUiColors() {
    return EnumVal<UiColors>(Registrar::GetSettingsInt(kSettingUiColors));
}

void Config::SetUiColors(UiColors colors) {
    Registrar::SetSettingsInt(kSettingUiColors, EnumInt(colors));
}

UiLanguage Config::GetUiLanguage() {
    return EnumVal<UiLanguage>(Registrar::GetSettingsInt(kSettingUiLanguage));
}

void Config::SetUiLanguage(UiLanguage lang) {
    Registrar::SetSettingsInt(kSettingUiLanguage, EnumInt(lang));
}

int Config::GetUiSize() {
    return Registrar::GetSettingsInt(kSettingUiSize);
}

void Config::SetUiSize(int size) {
    Registrar::SetSettingsInt(kSettingUiSize, size);
}

Hotkey Config::GetOnOffHotkey() {
    return EnumVal<Hotkey>(Registrar::GetSettingsInt(kSettingOnOffHotkey));
}

void Config::SetOnOffHotkey(Hotkey key) {
    Registrar::SetSettingsInt(kSettingOnOffHotkey, EnumInt(key));
}

Hotkey Config::GetInputModeHotkey() {
    auto i = Registrar::GetSettingsInt(kSettingInputModeHotkey);
    return i ? EnumVal<Hotkey>(i) : Hotkey::CtrlBacktick;
}

void Config::SetInputModeHotkey(Hotkey key) {
    Registrar::SetSettingsInt(kSettingInputModeHotkey, EnumInt(key));
}

} // namespace khiin::win32
