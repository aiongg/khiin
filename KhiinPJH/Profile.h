#pragma once

namespace Khiin {

struct Profile {
    static const GUID textServiceGuid();
    static const GUID languageProfileGuid();
    static const LCID langId();
    static const int displayNameIndex();
};

} // namespace Khiin
