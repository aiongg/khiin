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

constexpr auto kClsidPrefix = std::wstring_view(L"CLSID\\");
constexpr auto kInprocServer32 = std::wstring_view(L"InprocServer32");
constexpr auto kThreadingModel = std::wstring_view(L"ThreadingModel");
constexpr auto kApartmentModel = std::wstring_view(L"Apartment");
constexpr auto kClsidDescription = std::wstring_view(L"Khiin Taiwanese IME");
constexpr auto kSettingsPath = std::wstring_view(L"Software\\Khiin PJH");

constexpr auto wcharSize(std::wstring_view wstr) {
    return static_cast<uint32_t>((wstr.size() + 1) * sizeof(wchar_t));
}

} // namespace

void Registrar::RegisterComServer(std::wstring modulePath) {
    auto hkeypath = std::wstring(kClsidPrefix) + kTextServiceGuidString;
    auto hkey = registry_key();

    // Add our CLSID to HKEY_CLASSES_ROOT/CLSID/
    check_win32(::RegCreateKeyEx(HKEY_CLASSES_ROOT, hkeypath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE,
                                 nullptr, hkey.put(), nullptr));

    ::RegDeleteValue(hkey.get(), nullptr);

    // Set the description
    check_win32(::RegSetValueEx(hkey.get(), nullptr, 0, REG_SZ,
                                reinterpret_cast<BYTE const *>(kClsidDescription.data()),
                                wcharSize(kClsidDescription)));

    auto inprocserver32key = registry_key();

    // Add the InprocServer32 sub-key
    check_win32(::RegCreateKeyEx(hkey.get(), kInprocServer32.data(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE,
                                 nullptr, inprocserver32key.put(), nullptr));

    // Set the DLL module path
    check_win32(::RegSetValueEx(inprocserver32key.get(), nullptr, 0, REG_SZ,
                                reinterpret_cast<BYTE const *>(modulePath.c_str()), wcharSize(modulePath)));

    auto apartment = std::wstring(L"Apartment");
    check_win32(::RegSetValueEx(inprocserver32key.get(), kThreadingModel.data(), 0, REG_SZ,
                                reinterpret_cast<BYTE const *>(kApartmentModel.data()), wcharSize(kApartmentModel)));

    inprocserver32key.close();
    hkey.close();
}

void Registrar::UnregisterComServer() {
    auto hkeypath = std::wstring(kClsidPrefix) + kTextServiceGuidString;
    check_win32(::RegDeleteTreeW(HKEY_CLASSES_ROOT, hkeypath.c_str()));
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

std::wstring Registrar::GetSettingsString(std::wstring_view name) {
    auto rkey = registry_key();
    check_win32(::RegOpenKeyEx(HKEY_CURRENT_USER, kSettingsPath.data(), 0, KEY_READ, rkey.put()));
    DWORD data_size{};
    check_win32(::RegGetValue(rkey.get(), NULL, name.data(), RRF_RT_REG_SZ, nullptr, nullptr, &data_size));
    std::wstring data;
    data.resize(data_size / sizeof(wchar_t));
    check_win32(::RegGetValue(rkey.get(), NULL, name.data(), RRF_RT_REG_SZ, nullptr, &data[0], &data_size));
    data.resize(data_size / sizeof(wchar_t) - 1);
    rkey.close();
    return data;
}

int Registrar::GetSettingsInt(std::wstring_view name) {
    auto rkey = registry_key();
    check_win32(::RegOpenKeyEx(HKEY_CURRENT_USER, kSettingsPath.data(), 0, KEY_READ, rkey.put()));
    DWORD data{};
    DWORD data_size = sizeof(DWORD);
    check_win32(::RegGetValue(rkey.get(), NULL, name.data(), RRF_RT_REG_DWORD, nullptr, &data, &data_size));
    rkey.close();
    return static_cast<int>(data);
}

void Registrar::SetSettingsInt(std::wstring_view name, int value) {
    auto rkey = registry_key();
    check_win32(::RegOpenKeyEx(HKEY_CURRENT_USER, kSettingsPath.data(), 0, KEY_READ, rkey.put()));
    DWORD data{};
    DWORD data_size = sizeof(DWORD);
    rkey.close();
    check_win32(::RegSetValueEx(rkey.get(), name.data(), 0, REG_DWORD, (LPBYTE)&data, data_size));
}

} // namespace khiin::win32::tip
