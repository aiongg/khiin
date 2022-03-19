#include "pch.h"

#include "Application.h"

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

#include "proto/proto.h"
#include "tip/Config.h"

#include "AppearanceProps.h"
#include "InputProps.h"
#include "Strings.h"
#include "UserProps.h"

namespace khiin::win32::settings {
namespace {
using namespace winrt;
using namespace khiin::proto;
using namespace khiin::win32::tip;

Application *g_app;
HMODULE g_module = NULL;
AppConfig *g_config = nullptr;
WNDPROC g_propsheet_original_wndproc = NULL;

const std::wstring kWindowCaption = L"起引設定 Khíín Siat-tēng";

void InitCommonCtrls() {
    auto icc = INITCOMMONCONTROLSEX{};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_LINK_CLASS | ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS;
    ::InitCommonControlsEx(&icc);
}

void FinalizeCallback() {
    Config::SaveToFile(g_module, g_config);
    Config::NotifyChanged();
    g_app->Reinitialize();
}

LRESULT CALLBACK PropsheetWndProcOverride(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto ret = ::CallWindowProc(g_propsheet_original_wndproc, hdlg, msg, wParam, lParam);

    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            [[fallthrough]];
        case ID_APPLY_NOW:
            FinalizeCallback();
            break;
        }
        break;
    }

    return ret;
}

int PropsheetCallback(HWND hdlg, UINT msg, LPARAM lParam) {
    switch (msg) {
    case PCSB_INITIALIZED: {
        ::SetWindowLong(hdlg, GWL_STYLE, ::GetWindowLong(hdlg, GWL_STYLE) | WS_MINIMIZEBOX);
        auto hmenu = GetSystemMenu(hdlg, FALSE);
        ::InsertMenu(hmenu, -1, MF_BYPOSITION | MF_STRING, SC_ICON, L"Minimize");
        g_propsheet_original_wndproc =
            (WNDPROC)::SetWindowLongPtr(hdlg, GWLP_WNDPROC, (LONG_PTR)PropsheetWndProcOverride);
        break;
    }
    case PCSB_PRECREATE:
        InitCommonControls();
        break;
    }
    return 0;
}

void LoadConfig() {
    g_config = AppConfig::default_instance().New();
    Config::LoadFromFile(g_module, g_config);
}

class ApplicationImpl : public Application {
  public:
    ApplicationImpl(HMODULE hmod) :
        m_display(AppearanceProps(this)), m_input(InputProps(this)), m_about(PropSheetPage(this)),
        m_user(UserProps(this)) {
        g_module = hmod;
    }

    virtual void Reinitialize() override {
        m_display.Reload();
        m_input.Reload();
        m_user.Reload();
        UpdateTitle();
    }

    virtual int ShowDialog() override {
        LoadConfig();
        UpdateTitle();

        auto pages = std::vector<HPROPSHEETPAGE>();

        pages.push_back(m_display.psp(g_module, IDD_APPEARANCETAB, g_config));
        pages.push_back(m_input.psp(g_module, IDD_INPUTTAB, g_config));
        pages.push_back(m_user.psp(g_module, IDD_DICTIONARYTAB, g_config));
        pages.push_back(m_about.psp(g_module, IDD_ABOUTTAB, g_config));

        PROPSHEETHEADER psh = {};
        psh.dwSize = sizeof(PROPSHEETHEADER);
        psh.dwFlags = PSH_NOCONTEXTHELP | PSH_USECALLBACK | PSH_USEICONID;
        psh.hwndParent = HWND_DESKTOP;
        psh.hInstance = g_module;
        psh.nPages = pages.size();
        psh.nStartPage = 0;
        psh.pfnCallback = PropsheetCallback;
        psh.phpage = pages.data();
        psh.pszCaption = m_title.data();
        psh.pszIcon = MAKEINTRESOURCE(IDI_KHIINSETTINGS);

        return ::PropertySheet(&psh);
    }

  private:
    void UpdateTitle() {
        m_title = Strings::T(IDS_WINDOW_CAPTION, Config::GetUiLanguage());
        auto hwnd = ::GetParent(m_display.hwnd());
        if (hwnd) {
            ::SetWindowText(hwnd, m_title.data());
        }
    }

    AppearanceProps m_display;
    InputProps m_input;
    PropSheetPage m_about;
    UserProps m_user;
    std::wstring m_title;
};

} // namespace
} // namespace khiin::win32::settings

INT_PTR DialogProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
    return FALSE;
}

bool BringExistingDialogToFront() {
    using namespace khiin::win32::settings;
    auto ret = false;
    auto titles = std::vector<std::wstring>();
    titles.push_back(Strings::T(IDS_WINDOW_CAPTION, khiin::win32::UiLanguage::English));
    titles.push_back(Strings::T(IDS_WINDOW_CAPTION, khiin::win32::UiLanguage::HanloTai));

    for (auto &title : titles) {
        HWND hwnd = ::FindWindow(NULL, title.data());
        if (hwnd != NULL) {
            HWND cur = ::GetForegroundWindow();
            DWORD tid = ::GetCurrentThreadId();
            DWORD pid = ::GetWindowThreadProcessId(cur, NULL);
            ::AttachThreadInput(pid, tid, TRUE);
            ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
            ::SetForegroundWindow(hwnd);
            ::SetFocus(hwnd);
            ::SetActiveWindow(hwnd);
            ::AttachThreadInput(pid, tid, FALSE);
            ::ShowWindow(hwnd, SW_RESTORE);

            //::BringWindowToTop(hwnd);
            ret = true;
            break;
        }
    }

    return ret;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (BringExistingDialogToFront()) {
        return 1;
    }

    using namespace khiin::win32::settings;
    g_app = new ApplicationImpl(hInstance);
    return g_app->ShowDialog();
}
