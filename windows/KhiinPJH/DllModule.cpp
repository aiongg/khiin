#include "pch.h"

#include "DllModule.h"

#include "BaseWindow.h"
#include "EngineController.h"
#include "KhiinClassFactory.h"
#include "Registrar.h"
#include "TextService.h"

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
        khiin::win32::TextServiceFactory::OnDllProcessAttach(instance);
        khiin::win32::WindowSetup::OnDllProcessAttach(instance);
        khiin::win32::EngineControllerFactory::OnDllProcessAttach(instance);
        moduleHandle = instance;
        return TRUE;
    }

    static BOOL OnDllProcessDetach(HINSTANCE instance, bool process_shutdown) {
        khiin::win32::TextServiceFactory::OnDllProcessDetach(instance);
        khiin::win32::WindowSetup::OnDllProcessDetach(instance);
        khiin::win32::EngineControllerFactory::OnDllProcessDetach(instance);
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

        if (rclsid == __uuidof(khiin::win32::KhiinClassFactory)) {
            return winrt::make<khiin::win32::KhiinClassFactory>()->QueryInterface(riid, ppv);
        }

        return winrt::hresult_class_not_available().to_abi();
    } catch (...) {
        return winrt::to_hresult();
    }
}

STDMETHODIMP DllUnregisterServer() {
    khiin::win32::Registrar::unregisterCategories();
    khiin::win32::Registrar::unregisterProfiles();
    khiin::win32::Registrar::unregisterComServer();

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
        khiin::win32::Registrar::registerComServer(dllPath);
        khiin::win32::Registrar::registerProfiles(dllPath);
        khiin::win32::Registrar::registerCategories();
    } catch (...) {
        DllUnregisterServer();
        return winrt::to_hresult();
    }

    return S_OK;
}

LONG WINAPI TopLevelExceptionFilter(LPEXCEPTION_POINTERS e) {
    // When the IME crashes during initialization for any reason,
    // Windows continues to try loading the IME into every process
    // that enters the foreground after the previous one crashed.
    // Here we unregister the IME entirely to prevent that from happening.
    // You must re-register after a crash.
    DllUnregisterServer();
    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        ::SetUnhandledExceptionFilter(TopLevelExceptionFilter);
        ::DisableThreadLibraryCalls(hModule);
        return ModuleImpl::OnDllProcessAttach(hModule, lpReserved != nullptr);
    case DLL_PROCESS_DETACH:
        return ModuleImpl::OnDllProcessDetach(hModule, lpReserved != nullptr);
    }
    return TRUE;
}

namespace khiin::win32 {

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

} // namespace khiin::win32
