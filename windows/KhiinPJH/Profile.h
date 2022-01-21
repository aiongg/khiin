#pragma once

namespace khiin::win32 {

struct Profile {
    static inline const GUID textServiceGuid // 829893f6-728d-11ec-8c6e-e0d46491b35a
        {0x829893f6, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

    static inline const GUID languageProfileGuid // 829893f7-728d-11ec-8c6e-e0d46491b35a
        {0x829893f7, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

    static inline const LCID langId = ::LocaleNameToLCID(L"en-US", 0);

    static inline const int displayNameIndex = IDS_TEXT_SERVICE_DISPLAY_NAME;
};

} // namespace khiin::win32
