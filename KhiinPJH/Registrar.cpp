#include "pch.h"

#include "Profile.h"
#include "Registrar.h"

namespace Khiin {

const static auto supportedCategories = std::vector<GUID>{
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,    // It supports inline input.
    GUID_TFCAT_TIPCAP_COMLESS,              // It's a COM-Less module.
    GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT, // It supports input mode.
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED,     // It supports UI less mode.
    GUID_TFCAT_TIP_KEYBOARD,                // It's a keyboard input method.
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,     // It supports Metro mode.
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,       // It supports Win8 systray.
};

struct registry_traits {
    using type = HKEY;

    static void close(type value) noexcept {
        WINRT_VERIFY_(ERROR_SUCCESS, ::RegCloseKey(value));
    }

    static constexpr type invalid() noexcept {
        return nullptr;
    }
};

using registry_key = winrt::handle_type<registry_traits>;

auto textServiceGuidStr() {
    auto guid = std::wstring(39, L'?');
    auto guidlen = ::StringFromGUID2(Profile::textServiceGuid(), &guid[0], 64);
    if (!guidlen) {
        throw E_INVALIDARG;
    }
    return guid;
}

constexpr auto clsidPrefix = std::wstring_view(L"CLSID\\");
constexpr auto inprocServer32 = std::wstring_view(L"InprocServer32");
constexpr auto threadingModel = std::wstring_view(L"ThreadingModel");
constexpr auto apartmentModel = std::wstring_view(L"Apartment");
constexpr auto clsidDescription = std::wstring_view(L"Khiin Taiwanese IME");

constexpr auto wcharSize(std::wstring_view wstr) {
    return static_cast<uint32_t>((wstr.size() + 1) * sizeof(wchar_t));
}

void Registrar::registerComServer(std::wstring modulePath) {
    auto hkeypath = std::wstring(clsidPrefix) + textServiceGuidStr();
    auto hkey = registry_key();

    // Add our CLSID to HKEY_CLASSES_ROOT/CLSID/
    winrt::check_win32(::RegCreateKeyEx(HKEY_CLASSES_ROOT, hkeypath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE, nullptr, hkey.put(), nullptr));

    ::RegDeleteValue(hkey.get(), nullptr);

    // Set the description
    winrt::check_win32(::RegSetValueEx(hkey.get(), nullptr, 0, REG_SZ,
                                       reinterpret_cast<BYTE const *>(clsidDescription.data()),
                                       wcharSize(clsidDescription)));

    auto inprocserver32key = registry_key();

    // Add the InprocServer32 sub-key
    winrt::check_win32(::RegCreateKeyEx(hkey.get(), inprocServer32.data(), 0, nullptr, REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE, nullptr, inprocserver32key.put(), nullptr));

    // Set the DLL module path
    winrt::check_win32(::RegSetValueEx(inprocserver32key.get(), nullptr, 0, REG_SZ,
                                       reinterpret_cast<BYTE const *>(modulePath.c_str()), wcharSize(modulePath)));

    auto apartment = std::wstring(L"Apartment");
    winrt::check_win32(::RegSetValueEx(inprocserver32key.get(), threadingModel.data(), 0, REG_SZ,
                                       reinterpret_cast<BYTE const *>(apartmentModel.data()),
                                       wcharSize(apartmentModel)));

    inprocserver32key.close();
    hkey.close();
}

void Registrar::unregisterComServer() {
    auto hkeypath = std::wstring(clsidPrefix) + textServiceGuidStr();
    winrt::check_win32(::RegDeleteTreeW(HKEY_CLASSES_ROOT, hkeypath.c_str()));
    return;
}

void Registrar::registerProfiles(std::wstring modulePath) {
    auto inputProcessorProfiles = winrt::com_ptr<ITfInputProcessorProfiles>();

    winrt::check_hresult(::CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                                            IID_ITfInputProcessorProfiles, inputProcessorProfiles.put_void()));

    winrt::check_hresult(inputProcessorProfiles->Register(Profile::textServiceGuid()));

    winrt::check_hresult(inputProcessorProfiles->AddLanguageProfile(
        Profile::textServiceGuid(), Profile::langId(), Profile::languageProfileGuid(), clsidDescription.data(),
        wcharSize(clsidDescription), /* TODO: icon file */ NULL, NULL, NULL));

    if (auto profilesEx = inputProcessorProfiles.try_as<ITfInputProcessorProfilesEx>(); profilesEx) {
        winrt::check_hresult(profilesEx->SetLanguageProfileDisplayName(
            Profile::textServiceGuid(), Profile::langId(), Profile::languageProfileGuid(), modulePath.data(),
            wcharSize(modulePath), Profile::displayNameIndex()));
    }
}

void Registrar::unregisterProfiles() {
    auto inputProcessorProfiles = winrt::com_ptr<ITfInputProcessorProfiles>();
    winrt::check_hresult(::CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                                            IID_ITfInputProcessorProfiles, inputProcessorProfiles.put_void()));

    winrt::check_hresult(inputProcessorProfiles->Unregister(Profile::textServiceGuid()));
}

void Registrar::registerCategories() {
    auto categoryMgr = winrt::com_ptr<ITfCategoryMgr>();
    winrt::check_hresult(::CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,
                                            categoryMgr.put_void()));
    auto guid = Profile::textServiceGuid();

    for (auto &category : supportedCategories) {
        categoryMgr->RegisterCategory(guid, category, guid);
    }
}

void Registrar::unregisterCategories() {
    auto categoryMgr = winrt::com_ptr<ITfCategoryMgr>();
    winrt::check_hresult(::CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,
                                            categoryMgr.put_void()));
    auto guid = Profile::textServiceGuid();

    for (auto &category : supportedCategories) {
        categoryMgr->UnregisterCategory(guid, category, guid);
    }
}

HRESULT Registrar::getProfileEnabled(BOOL *enabled) {
    // TODO
    return E_NOTIMPL;
}

HRESULT Registrar::setProfileEnabled(BOOL enable) {
    // TODO
    return E_NOTIMPL;
}

} // namespace Khiin
