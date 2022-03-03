#pragma once

#include "PropSheet.h"

namespace khiin::win32::settings {

class AppearanceProps : public PropSheet {
  public:
    virtual void Initialize() override;
    virtual void Finalize() override;
};

} // namespace khiin::win32::settings