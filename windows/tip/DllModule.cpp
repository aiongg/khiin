#include "pch.h"

#include "DllModule.h"

#include "BaseWindow.h"
#include "EngineController.h"
#include "KhiinClassFactory.h"
#include "Logger.h"
#include "Registrar.h"
#include "TextService.h"
#include "Utils.h"

namespace {
namespace fs = std::filesystem;
using namespace khiin::win32;
using namespace khiin::win32::tip;

fs::path GetTempFolder() {
    wchar_t tmp[MAX_PATH];
    ::GetTempPath(MAX_PATH, tmp);
    auto path = fs::path(Utils::Narrow(std::wstring(tmp)));
    return path;
}

std::atomic_int count;

class ModuleImpl {
  public:
    static void AddRef() {
        ++count;
        KHIIN_TRACE("AddRef: count={}", count);
    }

    static void Release() {
        --count;
        KHIIN_TRACE("Release: count={}", count);
    }

    static bool IsUnloaded() {
        KHIIN_TRACE("IsUnloaded: unloaded={}", unloaded);
        return unloaded;
    }

    static bool CanUnload() {
        KHIIN_TRACE("CanUnload: count={}", count);
        return count <= 0;
    }

    static BOOL OnDllProcessAttach(HINSTANCE instance, bool static_loading) {
        moduleHandle = instance;
        Logger::Initialize(GetTempFolder());
        TextService::OnDllProcessAttach(instance);
        WindowSetup::OnDllProcessAttach(instance);
        EngineController::OnDllProcessAttach(instance);
        return TRUE;
    }

    static BOOL OnDllProcessDetach(HINSTANCE instance, bool process_shutdown) {
        KHIIN_TRACE("Shutting down...");
        TextService::OnDllProcessDetach(instance);
        WindowSetup::OnDllProcessDetach(instance);
        EngineController::OnDllProcessDetach(instance);
        Logger::Uninitialize();
        moduleHandle = nullptr;
        unloaded = true;
        return TRUE;
    }

    static HMODULE module_handle() {
        return moduleHandle;
    }

    static std::wstring module_path() {
        auto path = std::wstring(MAX_PATH, '?');
        auto pathsize = ::GetModuleFileName(ModuleImpl::module_handle(), &path[0], MAX_PATH);
        path.resize(static_cast<size_t>(pathsize));
        return path;
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
    KHIIN_TRACE("");
    try {
        *ppv = nullptr;

        if (rclsid == __uuidof(KhiinClassFactory)) {
            return winrt::make<KhiinClassFactory>()->QueryInterface(riid, ppv);
        }

        return winrt::hresult_class_not_available().to_abi();
    } catch (...) {
        return winrt::to_hresult();
    }
}

STDMETHODIMP DllUnregisterServer() {
    Registrar::UnregisterCategories();
    Registrar::UnregisterProfiles();
    Registrar::UnregisterComServer();

    return S_OK;
}

// Called when running regsvr32.exe
// 1. Register this DLL as a COM server;
// 2. Register this COM server as a TSF text m_service, and;
// 3. Register this text m_service as a TSF text-input processor.
STDMETHODIMP DllRegisterServer() {
    MessageBox(NULL, (LPCWSTR)L"Waiting for debugger...", (LPCWSTR)L"OK", MB_DEFBUTTON2);

    auto dllPath = ModuleImpl::module_path();

    try {
        using namespace khiin::win32::tip;
        Registrar::RegisterComServer(dllPath);
        Registrar::RegisterProfiles(dllPath);
        Registrar::RegisterCategories();
        Registrar::GetSettingsString(L"");
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
    try {
        KHIIN_CRITICAL("Crashing... ({})", e->ExceptionRecord->ExceptionCode);
    } catch (...) {
        // Do nothing
    }
    DllUnregisterServer();
    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // MessageBox(NULL, (LPCWSTR)L"Waiting for debugger...", (LPCWSTR)L"OK", MB_DEFBUTTON2);

    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        //::SetUnhandledExceptionFilter(TopLevelExceptionFilter);
        ::DisableThreadLibraryCalls(hModule);
        return ModuleImpl::OnDllProcessAttach(hModule, lpReserved != nullptr);
    case DLL_PROCESS_DETACH:
        if (lpReserved != nullptr) {
            break;
        }
        return ModuleImpl::OnDllProcessDetach(hModule, lpReserved != nullptr);
    }
    return TRUE;
}

namespace khiin::win32::tip {

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

} // namespace khiin::win32::tip
