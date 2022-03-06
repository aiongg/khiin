#include "pch.h"

#include "KhiinSettings.h"

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

#include <KhiinPJH/Config.h>

#include "AppearanceProps.h"
#include "InputProps.h"

namespace khiin::win32::settings {
namespace {
using namespace winrt;
using namespace khiin::messages;

HMODULE g_module = NULL;
AppConfig *g_config = nullptr;
WNDPROC g_propsheet_original_wndproc = NULL;

void InitCommonCtrls() {
    auto icc = INITCOMMONCONTROLSEX{};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_LINK_CLASS | ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS;
    ::InitCommonControlsEx(&icc);
}

void FinalizeCallback() {
    Config::SaveToFile(g_module, g_config);
    Config::NotifyChanged();
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
    case PCSB_INITIALIZED:
        g_propsheet_original_wndproc =
            (WNDPROC)::SetWindowLongPtr(hdlg, GWLP_WNDPROC, (LONG_PTR)PropsheetWndProcOverride);
        break;
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

class KhiinSettingsImpl : KhiinSettings {
  public:
    KhiinSettingsImpl(HMODULE hmod) {
        g_module = hmod;
    }

    virtual UiLanguage uilang() override {
        return m_lang;
    }

    virtual void set_uilang(UiLanguage lang) override {
        m_lang = lang;
    }

    virtual messages::AppConfig *config() override {
        return m_config;
    }

    int ShowDialog() {
        LoadConfig();

        auto pages = std::vector<HPROPSHEETPAGE>();

        auto appearance_props = AppearanceProps(this);
        auto input_props = InputProps(this);
        auto about = PropSheet(this);

        pages.push_back(appearance_props.psp(g_module, IDD_APPEARANCETAB, g_config));
        pages.push_back(input_props.psp(g_module, IDD_INPUTTAB, g_config));
        pages.push_back(about.psp(g_module, IDD_ABOUTTAB, g_config));

        PROPSHEETHEADER psh = {};
        psh.dwSize = sizeof(PROPSHEETHEADER);
        psh.dwFlags = PSH_NOCONTEXTHELP | PSH_USECALLBACK | PSH_USEICONID;
        psh.hwndParent = HWND_DESKTOP;
        psh.hInstance = g_module;
        psh.nPages = pages.size();
        psh.nStartPage = 0;
        psh.pfnCallback = PropsheetCallback;
        psh.phpage = pages.data();
        psh.pszCaption = L"起引設定 Khíín Siat-tēng";
        psh.pszIcon = MAKEINTRESOURCE(IDI_KHIINSETTINGS);

        return ::PropertySheet(&psh);
    }

  private:
    AppConfig *m_config;
    UiLanguage m_lang = UiLanguage::NotSet;
};

} // namespace
} // namespace khiin::win32::settings

INT_PTR DialogProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
    return FALSE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    auto app = khiin::win32::settings::KhiinSettingsImpl(hInstance);
    return app.ShowDialog();
}
