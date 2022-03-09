#pragma once

namespace khiin::win32::tip {

struct Registrar {
    static void RegisterComServer(std::wstring modulePath);

    // Unregisters the DLL from registry.
    static void UnregisterComServer();

    // Registers this COM server to the profile store for input processors.
    // The caller is responsible for initializing COM before call this function.
    static void RegisterProfiles(std::wstring modulePath);

    // Unregisters this COM server from the text service framework.
    // The caller is responsible for initializing COM before call this function.
    static void UnregisterProfiles();

    // Retrieves the category manager for text input processors, and
    // registers this module as a keyboard and a display attribute provider.
    // The caller is responsible for initializing COM before call this function.
    static void RegisterCategories();

    // Retrieves the category manager for text input processors, and unregisters
    // this keyboard module.
    // The caller is responsible for initializing COM before call this function.
    static void UnregisterCategories();

    // Retrieves if the text input processor profile is enabled or not.
    // This function sets FALSE to |enable| if the profile is not installed.
    // The caller is responsible for initializing COM before call this function.
    static HRESULT GetProfileEnabled(BOOL *enabled);

    // Enables or disables the text input processor profile.
    // The caller is responsible for initializing COM before call this function.
    static HRESULT SetProfileEnabled(BOOL enable);

    // Retrieve settings stored in the HKCU hive
    static std::wstring GetSettingsString(std::wstring_view name);
    static int GetSettingsInt(std::wstring_view name);
    static void SetSettingsInt(std::wstring_view name, int value);
};

} // namespace khiin::win32::tip
