#pragma once

#include "GuiWindow.h"

namespace khiin::win32 {

namespace tip {
struct TextService;
}

class PopupMenu : public GuiWindow {
  public:
    static PopupMenu *Create(tip::TextService *pservice);
    virtual void Show(POINT pt) = 0;
};

} // namespace khiin::win32
