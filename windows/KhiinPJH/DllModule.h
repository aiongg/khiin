#pragma once

namespace khiin::win32 {

class DllModule {
  public:
    static void AddRef();
    static void Release();

    static bool IsUnloaded();
    static bool CanUnload();

    static HMODULE module_handle();
};

} // namespace khiin::win32
