#include "pch.h"

#include "BaseWindow.h"
#include "DllModule.h"

namespace {
volatile HMODULE g_module_handle = nullptr;
}

namespace khiin::win32 {

void WindowSetup::OnDllProcessAttach(HMODULE module) {
    DllModule::AddRef();
    g_module_handle = module;
}

void WindowSetup::OnDllProcessDetach(HMODULE module) {
    g_module_handle = nullptr;
    DllModule::Release();
}

HMODULE WindowSetup::hmodule() {
    return g_module_handle;
}

} // namespace khiin::win32
