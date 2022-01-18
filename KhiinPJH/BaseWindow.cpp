#include "pch.h"

#include "BaseWindow.h"

namespace Khiin {

HMODULE g_moduleHandle = NULL;

void WindowSetup::OnDllProcessAttach(HMODULE module) {
    g_moduleHandle = module;
}

} // namespace Khiin
