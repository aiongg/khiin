#include "pch.h"

#include "Registrar.h"

#include "Guids.h"
#include "Profile.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;

struct registry_traits {
    using type = HKEY;
    inline static void close(type value) noexcept {
        WINRT_VERIFY_(ERROR_SUCCESS, ::RegCloseKey(value));
    }
    inline static constexpr type invalid() noexcept {
        return nullptr;
    }
};

using registry_key = handle_type<registry_traits>;

const std::wstring kTextServiceGuidString = guids::String(guids::kTextService);

const auto supportedCategories = std::vector<GUID>{
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,    // It supports inline input.
    GUID_TFCAT_TIPCAP_COMLESS,              // It's a COM-Less module.
    GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT, // It supports input mode.
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED,     // It supports UI less mode.
    GUID_TFCAT_TIP_KEYBOARD,                // It's a keyboard input method.
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,     // It supports Metro mode.
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,       // It supports Win8 systray.
};

const std::wstring kClsidPrefix = L"CLSID\\";
const std::wstring kInprocServer32 = L"InprocServer32";
const std::wstring kThreadingModel = L"ThreadingModel";
const std::wstring kApartmentModel = L"Apartment";
const std::wstring kClsidDescription = L"Khiin Taiwanese IME";
const std::wstring kHkcuAppPath = L"Software\\Khiin PJH";
const std::wstring kSettingsPath = L"Software\\Khiin PJH\\Settings";

constexpr auto wcharSize(std::wstring_view wstr) {
    return static_cast<uint32_t>((wstr.size() + 1) * sizeof(wchar_t));
}

registry_key CreateKey(registry_key const &hkey, std::wstring const &subkey) {
    auto ret = registry_key();
    check_win32(::RegCreateKeyEx(hkey.get(), subkey.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                                 nullptr, ret.put(), nullptr));
    return ret;
}

void DeleteTree(registry_key const &hkey, std::wstring const &subkey) {
    ::RegDeleteTree(hkey.get(), subkey.data());
}

void DeleteAllValues(registry_key const &hkey) {
    ::RegDeleteValue(hkey.get(), nullptr);
}

int GetIntValue(registry_key const &key, std::wstring const &name) {
    DWORD data{};
    DWORD data_size = sizeof(data);
    if (ERROR_SUCCESS != ::RegGetValue(key.get(), NULL, name.c_str(), RRF_RT_REG_DWORD, nullptr, &data, &data_size)) {
        return 0;
    }
    return static_cast<int>(data);
}

std::wstring GetStringValue(registry_key const &key, std::wstring const &name) {
    DWORD data_size{};
    if (ERROR_SUCCESS != ::RegGetValue(key.get(), NULL, name.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &data_size)) {
        return std::wstring();
    }
    std::wstring data;
    data.resize(data_size / sizeof(wchar_t));
    if (ERROR_SUCCESS != ::RegGetValue(key.get(), NULL, name.c_str(), RRF_RT_REG_SZ, nullptr, &data[0], &data_size)) {
        return std::wstring();
    }

    data.resize(data_size / sizeof(wchar_t) - 1);
    return data;
}

void SetIntValue(registry_key const &key, std::wstring const &name, int value) {
    DWORD data = static_cast<DWORD>(value);
    DWORD data_size = sizeof(DWORD);
    check_win32(::RegSetValueEx(key.get(), name.data(), 0, REG_DWORD, (LPBYTE)&data, data_size));
}

void SetStringValue(registry_key const &hkey, std::wstring const &name, std::wstring const &value) {
    auto data = reinterpret_cast<BYTE const *>(value.data());
    auto data_size = wcharSize(value);
    check_win32(::RegSetValueEx(hkey.get(), name.data(), 0, REG_SZ, data, data_size));
}

registry_key ClassesRoot() {
    return registry_key(HKEY_CLASSES_ROOT);
}

registry_key CurrentUser() {
    return registry_key(HKEY_CURRENT_USER);
}

registry_key SettingsRoot() {
    auto hkcu = CurrentUser();
    return CreateKey(hkcu, kSettingsPath.data());
}

} // namespace

void Registrar::RegisterComServer(std::wstring modulePath) {
    auto subkeypath = std::wstring(kClsidPrefix) + kTextServiceGuidString;

    // Add our CLSID to HKEY_CLASSES_ROOT/CLSID/
    auto clsroot = registry_key(HKEY_CLASSES_ROOT);
    auto clskey = CreateKey(clsroot, subkeypath);
    DeleteAllValues(clskey);

    // Set the description
    SetStringValue(clskey, L"", kClsidDescription.data());

    // Add the InprocServer32 sub-key
    auto inprockey = CreateKey(clskey, kInprocServer32.data());

    // Set the DLL module path
    SetStringValue(inprockey, L"", modulePath);
    SetStringValue(inprockey, kThreadingModel.data(), kApartmentModel.data());
    inprockey.close();
    clskey.close();
    clsroot.close();
}

void Registrar::UnregisterComServer() {
    auto clsroot = registry_key(HKEY_CLASSES_ROOT);
    auto clskey = std::wstring(kClsidPrefix) + kTextServiceGuidString;
    // check_win32(::RegDeleteTree(HKEY_CLASSES_ROOT, clskey.c_str()));
    DeleteTree(clsroot, clskey);
    auto hkcuroot = registry_key(HKEY_CURRENT_USER);
    DeleteTree(hkcuroot, kHkcuAppPath.data());
    return;
}

void Registrar::RegisterProfiles(std::wstring modulePath) {
    auto inputProcessorProfiles = com_ptr<ITfInputProcessorProfiles>();

    check_hresult(::CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                                     IID_ITfInputProcessorProfiles, inputProcessorProfiles.put_void()));

    check_hresult(inputProcessorProfiles->Register(guids::kTextService));

    check_hresult(inputProcessorProfiles->AddLanguageProfile(guids::kTextService, static_cast<LANGID>(Profile::langId),
                                                             guids::kLanguageProfile, kClsidDescription.data(),
                                                             wcharSize(kClsidDescription), modulePath.data(), NULL, 0));

    if (auto profilesEx = inputProcessorProfiles.try_as<ITfInputProcessorProfilesEx>(); profilesEx) {
        check_hresult(profilesEx->SetLanguageProfileDisplayName(
            guids::kTextService, static_cast<LANGID>(Profile::langId), guids::kLanguageProfile, modulePath.data(),
            wcharSize(modulePath), Profile::displayNameIndex));
    }
}

void Registrar::UnregisterProfiles() {
    auto inputProcessorProfiles = com_ptr<ITfInputProcessorProfiles>();
    check_hresult(::CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                                     IID_ITfInputProcessorProfiles, inputProcessorProfiles.put_void()));

    check_hresult(inputProcessorProfiles->Unregister(guids::kTextService));
}

void Registrar::RegisterCategories() {
    auto categoryMgr = com_ptr<ITfCategoryMgr>();
    check_hresult(::CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,
                                     categoryMgr.put_void()));
    auto guid = guids::kTextService;

    for (auto &category : supportedCategories) {
        check_hresult(categoryMgr->RegisterCategory(guid, category, guid));
    }
}

void Registrar::UnregisterCategories() {
    auto categoryMgr = com_ptr<ITfCategoryMgr>();
    check_hresult(::CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,
                                     categoryMgr.put_void()));
    auto guid = guids::kTextService;

    for (auto &category : supportedCategories) {
        check_hresult(categoryMgr->UnregisterCategory(guid, category, guid));
    }
}

HRESULT Registrar::GetProfileEnabled(BOOL *enabled) {
    // TODO
    return E_NOTIMPL;
}

HRESULT Registrar::SetProfileEnabled(BOOL enable) {
    // TODO
    return E_NOTIMPL;
}

std::wstring Registrar::GetSettingsString(std::wstring const &name) {
    auto hkey = SettingsRoot();
    return GetStringValue(hkey, name);
}

void Registrar::SetSettingsString(std::wstring const &name, std::wstring const &value) {
    auto hkey = SettingsRoot();
    return SetStringValue(hkey, name, value);
}

int Registrar::GetSettingsInt(std::wstring const &name) {
    auto hkey = SettingsRoot();
    return GetIntValue(hkey, name);
}

void Registrar::SetSettingsInt(std::wstring const &name, int value) {
    auto key = SettingsRoot();
    return SetIntValue(key, name, value);
}

} // namespace khiin::win32::tip
