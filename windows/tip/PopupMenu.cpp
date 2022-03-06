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
        service.copy_from(pservice);
        BaseWindow::Create(NULL, 0, WS_EX_NOACTIVATE);
    }

    virtual std::wstring const &ClassName() const override {
        return kMenuWindowClassName;
    }

    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override {
        switch (msg) {
        case WM_COMMAND:
            KHIIN_DEBUG("Command received");
            break;
        case WM_MEASUREITEM: {
            auto mis = reinterpret_cast<MEASUREITEMSTRUCT *>(lparam);
            mis->itemID = 0;
            mis->itemWidth = 200;
            mis->itemHeight = 30;
            return TRUE;
        }
        case WM_DRAWITEM: {
            auto dis = reinterpret_cast<DRAWITEMSTRUCT *>(lparam);
            auto fac = Graphics::CreateD2D1Factory();
            auto wfac = Graphics::CreateDwriteFactory();
            auto trg = Graphics::CreateDCRenderTarget(fac);
            trg->BindDC(dis->hDC, &dis->rcItem);
            trg->BeginDraw();
            trg->Clear(D2D1::ColorF(D2D1::ColorF::White));
            auto tfmt = com_ptr<IDWriteTextFormat>();
            wfac->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
                                   DWRITE_FONT_STRETCH_NORMAL, 16, L"en-us", tfmt.put());
            auto tlyt = com_ptr<IDWriteTextLayout>();
            wfac->CreateTextLayout(L"Test", 5, tfmt.get(), 100, 30, tlyt.put());
            auto brs = com_ptr<ID2D1SolidColorBrush>();
            trg->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), brs.put());
            trg->DrawTextLayout(D2D1::Point2F(0, 0), tlyt.get(), brs.get());
            trg->EndDraw();
            return TRUE;
        }
        }

        if (msg == WM_COMMAND) {
        }
        return ::DefWindowProc(m_hwnd, msg, wparam, lparam);
    }

    virtual int Show(POINT pt) override {
        auto menu = ::CreatePopupMenu();

        auto hicon =
            static_cast<HICON>(::LoadImage(service->hmodule(), MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, 0));
        ICONINFO iconinfo;
        ::GetIconInfo(hicon, &iconinfo);
        HBITMAP hbitmap = iconinfo.hbmColor;

        MENUITEMINFO info1;
        info1.cbSize = sizeof(MENUITEMINFO);
        info1.fMask = MIIM_FTYPE; // | MIIM_STRING | MIIM_STATE;
        info1.fType = MFT_OWNERDRAW;
        // info.fState = MFS_CHECKED;
        // std::wstring text = L"Test";
        // info.dwTypeData = &text[0];
        // info.hbmpItem = hbitmap;

        MENUITEMINFO info2;
        info2.cbSize = sizeof(MENUITEMINFO);
        info2.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_STATE | MIIM_BITMAP;
        info2.fType = MFT_RADIOCHECK;
        info2.fState = MFS_CHECKED;
        std::wstring text = L"Test1";
        info2.dwTypeData = &text[0];
        info2.hbmpItem = hbitmap;
        ::InsertMenuItem(menu, 0, TRUE, &info2);

        MENUITEMINFO info3;
        info3.cbSize = sizeof(MENUITEMINFO);
        info3.fMask = MIIM_STRING;
        std::wstring text1 = L"Test2 some very longer string";
        info3.dwTypeData = &text1[0];
        ::InsertMenuItem(menu, 0, TRUE, &info3);
        //::InsertMenuItem(menu, 0, TRUE, &info1);

        // if (ret == 0) {
        //    auto err = ::GetLastError();
        //    KHIIN_DEBUG("Error code: {:x}", err);
        //}

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
