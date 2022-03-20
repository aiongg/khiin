#pragma once

#include "PropSheetPage.h"

namespace khiin::win32::settings {

class Application;

class UserProps : public PropSheetPage {
  public:
    UserProps(Application *app);
    void Initialize() override;
    void Finalize() override;
    INT_PTR DlgProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

  private:
    void HandleClearUserData();
    void HandleChooseUserDictionaryFile();
    void HandleUnloadUserDictionaryFile();
};

} // namespace khiin::win32::settings
