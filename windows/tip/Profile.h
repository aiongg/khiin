#pragma once

#include "Guids.h"

namespace khiin::win32::tip {

struct Profile {
    static inline const LANGID langId = static_cast<LANGID>(::LocaleNameToLCID(L"zh-TW", 0));

    static inline constexpr int displayNameIndex = IDS_TEXT_SERVICE_DISPLAY_NAME;
};

} // namespace khiin::win32
