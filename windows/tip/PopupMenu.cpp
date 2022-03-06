#include "pch.h"

#include "PopupMenu.h"

#include "TextService.h"

namespace khiin::win32 {
namespace {
using namespace winrt;

const std::wstring kMenuWindowClassName = L"PopupMenuWindow";

class PopupMenuImpl : public PopupMenu {
  public:
    PopupMenuImpl(TextService *pservice) {
        service.copy_from(pservice);
        BaseWindow::Create(NULL, 0, WS_EX_NOACTIVATE);
    }

    virtual std::wstring const &ClassName() const override {
        return kMenuWindowClassName;
    }

    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override {
        if (msg == WM_COMMAND) {
            KHIIN_DEBUG("Command received");
        }
        return ::DefWindowProc(m_hwnd, msg, wparam, lparam);
    }

    virtual int Show(POINT pt) override {
        auto menu = ::CreatePopupMenu();

        MENUITEMINFO info;
        info.cbSize = sizeof(MENUITEMINFO);
        info.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_STATE;
        info.fType = MFT_RADIOCHECK;
        info.fState = MFS_CHECKED;
        std::wstring text = L"Test";
        info.dwTypeData = &text[0];

        auto ret = ::InsertMenuItem(menu, 0, TRUE, &info);
        KHIIN_DEBUG("Insert menu item: {}", ret);

        if (ret == 0) {
            auto err = ::GetLastError();
            KHIIN_DEBUG("Error code: {:x}", err);
        }

        return ::TrackPopupMenuEx(menu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hwnd, NULL);
    }

  private:
    com_ptr<TextService> service = nullptr;
};
} // namespace

PopupMenu *PopupMenu::Create(TextService *pservice) {
    return new PopupMenuImpl(pservice);
}

} // namespace khiin::win32
