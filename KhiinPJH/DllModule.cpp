#include "pch.h"

#include "DllModule.h"

#include "BaseWindow.h"
#include "CandidateWindow.h"
#include "KhiinClassFactory.h"
#include "Registrar.h"

namespace {

std::atomic_int count;

class ModuleImpl {
  public:
    static void AddRef() {
        ++count;
    }

    static void Release() {
        --count;
    }

    static bool IsUnloaded() {
        return unloaded;
    }

    static bool CanUnload() {
        D("__FUNCTIONW__", " Count: ", count);
        return count <= 0;
    }

    static BOOL OnDllProcessAttach(HINSTANCE instance, bool static_loading) {
        Khiin::WindowSetup::OnDllProcessAttach(instance);
        moduleHandle = instance;
        return TRUE;
    }

    static BOOL OnDllProcessDetach(HINSTANCE instance, bool process_shutdown) {
        Khiin::WindowSetup::OnDllProcessDetach(instance);
        moduleHandle = nullptr;
        unloaded = true;
        return TRUE;
    }

    static HMODULE module_handle() {
        return moduleHandle;
    }

  private:
    static HMODULE moduleHandle;
    static bool unloaded;
};

HMODULE ModuleImpl::moduleHandle = nullptr;
bool ModuleImpl::unloaded = false;

} // namespace

__control_entrypoint(DllExport) STDAPI DllCanUnloadNow(void) {
    if (!ModuleImpl::CanUnload()) {
        return S_FALSE;
    }

    return S_OK;
}

_Check_return_ STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR *ppv) {
    D("DllGetClassObject");
    try {
        *ppv = nullptr;

        if (rclsid == __uuidof(Khiin::KhiinClassFactory)) {
            return winrt::make<Khiin::KhiinClassFactory>()->QueryInterface(riid, ppv);
        }

        return winrt::hresult_class_not_available().to_abi();
    } catch (...) {
        return winrt::to_hresult();
    }
}

STDMETHODIMP DllUnregisterServer() {
    Khiin::Registrar::unregisterCategories();
    Khiin::Registrar::unregisterProfiles();
    Khiin::Registrar::unregisterComServer();

    return S_OK;
}

// Called when running regsvr32.exe
// 1. Register this DLL as a COM server;
// 2. Register this COM server as a TSF text service, and;
// 3. Register this text service as a TSF text-input processor.
STDMETHODIMP DllRegisterServer() {
    // MessageBox(NULL, (LPCWSTR)L"1", (LPCWSTR)L"OK", MB_DEFBUTTON2);

    auto dllPath = std::wstring(MAX_PATH, '?');
    auto pathsize = ::GetModuleFileName(ModuleImpl::module_handle(), &dllPath[0], MAX_PATH);
    dllPath.resize(static_cast<size_t>(pathsize));

    try {
        Khiin::Registrar::registerComServer(dllPath);
        Khiin::Registrar::registerProfiles(dllPath);
        Khiin::Registrar::registerCategories();
    } catch (...) {
        DllUnregisterServer();
        return winrt::to_hresult();
    }

    return S_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls(hModule);
        return ModuleImpl::OnDllProcessAttach(hModule, lpReserved != nullptr);
    case DLL_PROCESS_DETACH:
        return ModuleImpl::OnDllProcessDetach(hModule, lpReserved != nullptr);
    }
    return TRUE;
}

namespace Khiin {

void DllModule::AddRef() {
    ModuleImpl::AddRef();
}

void DllModule::Release() {
    ModuleImpl::Release();
}

bool DllModule::IsUnloaded() {
    return ModuleImpl::IsUnloaded();
}

bool DllModule::CanUnload() {
    return ModuleImpl::CanUnload();
}

HMODULE DllModule::module_handle() {
    return ModuleImpl::module_handle();
}

} // namespace Khiin
