#include "pch.h"

#include "BaseWindow.h"
#include "DllModule.h"

namespace khiin::win32 {

HMODULE g_moduleHandle = nullptr;

void WindowSetup::OnDllProcessAttach(HMODULE module) {
    DllModule::AddRef();
    g_moduleHandle = module;
}

void WindowSetup::OnDllProcessDetach(HMODULE module) {
    g_moduleHandle = nullptr;
    DllModule::Release();
}

} // namespace khiin::win32
