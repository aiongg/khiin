#pragma once

#include "PropSheet.h"

namespace khiin::win32::settings {

class KhiinSettings;

class AppearanceProps : public PropSheet {
  public:
    AppearanceProps(KhiinSettings *app);
    virtual void Initialize() override;
    virtual void Finalize() override;
};

} // namespace khiin::win32::settings
