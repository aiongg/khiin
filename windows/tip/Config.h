#pragma once

namespace khiin::proto {
class AppConfig;
}

namespace khiin::win32 {

class Config {
  public:
    static void LoadFromFile(HMODULE hmodule, proto::AppConfig *config);
    static void SaveToFile(HMODULE hmodule, proto::AppConfig *config);
    static void NotifyChanged();
};

namespace tip {
inline const GUID kConfigChangedCompartmentGuid // 829893fc-728d-11ec-8c6e-e0d46491b35a
    {0x829893fc, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

struct ConfigChangeListener {
    virtual void OnConfigChanged(proto::AppConfig *config) = 0;
};
} // namespace tip

} // namespace khiin::win32
