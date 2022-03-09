#include "pch.h"

#include "Config.h"

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>

#include <google/protobuf/util/json_util.h>

#include "Files.h"
#include "Guids.h"
#include "proto/proto.h"

namespace khiin::win32 {

namespace {
using namespace winrt;
using namespace proto;
namespace fs = std::filesystem;
using namespace khiin::win32::tip;

inline constexpr std::string_view kConfigFilename = "khiin_config.json";

} // namespace

UiLanguage Config::GetSystemLang() {
    if (auto lang = PRIMARYLANGID(::GetUserDefaultUILanguage());
        lang == LANG_CHINESE || lang == LANG_CHINESE_TRADITIONAL) {
        return UIL_TAI_HANLO;
    } else {
        return UIL_ENGLISH;
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

    if (config->appearance().ui_language() == UIL_UNSPECIFIED) {
        config->mutable_appearance()->set_ui_language(Config::GetSystemLang());
    }

    if (config->input_mode() == IM_UNSPECIFIED) {
        config->set_input_mode(IM_CONTINUOUS);
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

} // namespace khiin::win32
