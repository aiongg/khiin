#include "pch.h"

#include "Profile.h"

namespace Khiin {

static const GUID khiinClassFactory // 829893f6-728d-11ec-8c6e-e0d46491b35a
    {0x829893f6, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

static const GUID languageProfile // 829893f7-728d-11ec-8c6e-e0d46491b35a
    {0x829893f7, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

const GUID Profile::textServiceGuid() {
    return khiinClassFactory;
}

const GUID Profile::languageProfileGuid() {
    return languageProfile;
}

const LCID Profile::langId() {
    return ::LocaleNameToLCID(L"en-US", 0);
}

const int Profile::displayNameIndex() {
    return IDS_TEXT_SERVICE_DISPLAY_NAME;
}

} // namespace Khiin
