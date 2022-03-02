#include "pch.h"

#include "KhiinSettings.h"

#include <string>

namespace khiin::win32::settings {

struct PropSheetPage : PROPSHEETPAGE {
    intptr_t self;
};

template <typename DialogT>
class BaseDialog {
  public:
    static LRESULT CALLBACK StaticDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        PropSheetPage *psp = NULL;

        if (uMsg == WM_INITDIALOG) {
            psp = reinterpret_cast<PropSheetPage *>(lParam);
            ::SetWindowLongPtr(hwnd, DWLP_USER, reinterpret_cast<LONG_PTR>(psp));
        } else {
            psp = reinterpret_cast<PropSheetPage *>(::GetWindowLongPtr(hwnd, DWLP_USER));
        }

        if (psp) {
            auto self = reinterpret_cast<DialogT *>(psp->self);
            return self->DlgProc(hwnd, uMsg, wParam, lParam);
        } else {
            return FALSE;
        }
    }

  protected:
    virtual INT_PTR DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_INITDIALOG:
            return Initialize(hwnd);
        case WM_COMMAND:
            switch (wParam) {
            case IDOK:
                return Finalize(hwnd, IDOK);
            case IDCANCEL:
                return Finalize(hwnd, IDCANCEL);
            }
        }

        return FALSE; //::DefDlgProc(m_hwnd, msg, wParam, lParam);
    }

    virtual bool Initialize(HWND hwnd) {
        return false;
    }

    virtual bool Finalize(HWND hwnd, int command) {
        return ::EndDialog(hwnd, command);
    }
};

class Propsheet : public BaseDialog<Propsheet> {
  public:
    Propsheet(HINSTANCE instance, int template_id) : m_instance(instance), m_template_id(template_id) {
        m_psp.dwSize = sizeof(PropSheetPage);
        m_psp.hInstance = m_instance;
        m_psp.pszTemplate = MAKEINTRESOURCE(m_template_id);
        m_psp.pfnDlgProc = &StaticDlgProc;
        m_psp.pszTitle = L"Foo";
        m_psp.self = reinterpret_cast<intptr_t>(this);
    }

    PropSheetPage *psp() {
        return &m_psp;
    }

  private:
    PropSheetPage m_psp = {};
    HINSTANCE m_instance = NULL;
    int m_template_id = 0;
};

class AppearanceProps : public Propsheet {
  public:
    AppearanceProps(HINSTANCE instance, int template_id) : Propsheet(instance, template_id) {
        m_themes.push_back(L"光个");
        m_themes.push_back(L"暗个");
    }

    virtual bool Initialize(HWND hwnd) override {
        HWND theme = ::GetDlgItem(hwnd, IDC_COMBOBOX_THEME_COLOR);
        for (auto &s : m_themes) {
            ComboBox_AddString(theme, &s[0]);
        }
        return true;
    }

  private:
    std::vector<std::wstring> m_themes;
};

class KhiinSettings {
  public:
    KhiinSettings(HINSTANCE instance) : m_instance(instance){};

    int Run() {
        auto icc = INITCOMMONCONTROLSEX{};
        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_LINK_CLASS | ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS;
        ::InitCommonControlsEx(&icc);

        auto appearance_props = AppearanceProps(m_instance, IDD_APPEARANCE_PROPS);
        auto pages = std::vector<PropSheetPage *>();
        pages.push_back(appearance_props.psp());

        PROPSHEETHEADER psh = {};
        psh.dwSize = sizeof(PROPSHEETHEADER);
        psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP;
        psh.hwndParent = HWND_DESKTOP;
        psh.hInstance = m_instance;
        psh.nPages = 1;
        psh.nStartPage = 0;
        psh.ppsp = pages[0];
        psh.pszCaption = L"起引設定 Khíín Siat-tēng";

        auto result = ::PropertySheet(&psh);
        return result;
    }

  private:
    HINSTANCE m_instance;
};

} // namespace khiin::win32::settings

INT_PTR DialogProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
    return FALSE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    auto settings = khiin::win32::settings::KhiinSettings(hInstance);
    return settings.Run();
}
