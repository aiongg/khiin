#include "pch.h"

#include "BaseWindow.h"

namespace Khiin {

HMODULE g_moduleHandle = nullptr;

void WindowSetup::OnDllProcessAttach(HMODULE module) {
    g_moduleHandle = module;
}

void WindowSetup::OnDllProcessDetach(HMODULE module) {
    g_moduleHandle = nullptr;
}

} // namespace Khiin
