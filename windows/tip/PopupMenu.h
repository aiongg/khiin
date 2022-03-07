#pragma once
#include "BaseWindow.h"

#include "GuiWindow.h"

namespace khiin::win32 {

struct TextService;

class PopupMenu : public GuiWindow {
  public:
    static PopupMenu *Create(TextService *pservice);
    //virtual void Show(POINT pt) = 0;
    //virtual void Hide() = 0;

    //+---------------------------------------------------------------------------
    //
    // BaseWindow
    //
    //----------------------------------------------------------------------------

    //virtual std::wstring const &ClassName() const override = 0;
    //virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override = 0;
};

} // namespace khiin::win32
