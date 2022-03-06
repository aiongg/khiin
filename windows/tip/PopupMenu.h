#pragma once
#include "BaseWindow.h"

namespace khiin::win32 {

struct TextService;

class PopupMenu : public BaseWindow<PopupMenu> {
  public:
    static PopupMenu *Create(TextService *pservice);
    virtual std::wstring const &ClassName() const override = 0;
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override = 0;
    virtual int Show(POINT pt) = 0;
};

} // namespace khiin::win32
