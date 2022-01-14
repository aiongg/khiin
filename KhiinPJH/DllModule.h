#pragma once

namespace Khiin {

class DllModule {
  public:
    // Increases and decreases the reference count to this module.
    // This reference count is used for preventing Windows from unloading
    // this module.
    static LONG AddRef();
    static LONG Release();

    static bool IsUnloaded();
    static bool CanUnload();

    static HMODULE module_handle();
};

} // namespace Khiin