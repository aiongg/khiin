#pragma once

namespace Khiin {

class DllModule {
  public:
    static void AddRef();
    static void Release();

    static bool IsUnloaded();
    static bool CanUnload();

    static HMODULE module_handle();
};

} // namespace Khiin
