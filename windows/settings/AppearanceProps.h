#pragma once

#include "PropSheet.h"

namespace khiin::win32::settings {

class Application;

class AppearanceProps : public PropSheet {
  public:
    AppearanceProps(Application *app);
    virtual void Initialize() override;
    virtual void Finalize() override;
};

} // namespace khiin::win32::settings
