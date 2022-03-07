#pragma once
#include "BaseWindow.h"

#include "GuiWindow.h"

namespace khiin::win32 {

struct TextService;

class PopupMenu : public GuiWindow {
  public:
    static PopupMenu *Create(TextService *pservice);
    virtual void Show(POINT pt) = 0;
};

} // namespace khiin::win32
