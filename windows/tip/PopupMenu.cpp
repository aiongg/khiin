#include "pch.h"

#include "PopupMenu.h"

#include "Graphics.h"
#include "TextService.h"

namespace khiin::win32 {
namespace {
using namespace winrt;

const std::wstring kMenuWindowClassName = L"PopupMenuWindow";

class PopupMenuImpl : public PopupMenu {
  public:
    PopupMenuImpl(TextService *pservice) {
        m_service.copy_from(pservice);
        BaseWindow::Create(NULL, 0, WS_EX_NOACTIVATE);
    }

    virtual std::wstring const &ClassName() const override {
        return kMenuWindowClassName;
    }

    virtual void Show() override {
        ::SetWindowPos(m_hwnd, HWND_TOPMOST, 100, 100, 100, 100, SWP_NOACTIVATE | SWP_NOZORDER);
        ::ShowWindow(m_hwnd, SW_SHOWNA);
    }

    virtual void Hide() override {
        ::ShowWindow(m_hwnd, SW_HIDE);
    }

  private:
    void CalculateLayout() {}

    virtual void Render() override {
        m_target->Clear(D2D1::ColorF(D2D1::ColorF::Red));
    }

    com_ptr<TextService> m_service = nullptr;
    com_ptr<ID2D1SolidColorBrush> m_brush = nullptr;
    com_ptr<IDWriteTextFormat> m_textformat = nullptr;
};
} // namespace

PopupMenu *PopupMenu::Create(TextService *pservice) {
    return new PopupMenuImpl(pservice);
}

} // namespace khiin::win32
