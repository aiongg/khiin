#pragma once

#include "PropSheetPage.h"

namespace khiin::win32::settings {

class Application;

class InputProps : public PropSheetPage {
  public:
    InputProps(Application *app);
    virtual void Initialize() override;
    virtual void Finalize() override;
};

} // namespace khiin::win32::settings