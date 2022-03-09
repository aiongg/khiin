#pragma once

namespace khiin::win32::tip {

struct Registrar {
    static void registerComServer(std::wstring modulePath);

    // Unregisters the DLL from registry.
    static void unregisterComServer();

    // Registers this COM server to the profile store for input processors.
    // The caller is responsible for initializing COM before call this function.
    static void registerProfiles(std::wstring modulePath);

    // Unregisters this COM server from the text service framework.
    // The caller is responsible for initializing COM before call this function.
    static void unregisterProfiles();

    // Retrieves the category manager for text input processors, and
    // registers this module as a keyboard and a display attribute provider.
    // The caller is responsible for initializing COM before call this function.
    static void registerCategories();

    // Retrieves the category manager for text input processors, and unregisters
    // this keyboard module.
    // The caller is responsible for initializing COM before call this function.
    static void unregisterCategories();

    // Retrieves if the text input processor profile is enabled or not.
    // This function sets FALSE to |enable| if the profile is not installed.
    // The caller is responsible for initializing COM before call this function.
    static HRESULT getProfileEnabled(BOOL *enabled);

    // Enables or disables the text input processor profile.
    // The caller is responsible for initializing COM before call this function.
    static HRESULT setProfileEnabled(BOOL enable);
};

} // namespace khiin::win32
