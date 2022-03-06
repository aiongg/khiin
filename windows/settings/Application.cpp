#include "pch.h"

#include "Application.h"

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

#include "tip/Config.h"

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

UiLanguage GetSystemLang() {
    if (auto lang = PRIMARYLANGID(::GetUserDefaultUILanguage());
        lang == LANG_CHINESE || lang == LANG_CHINESE_TRADITIONAL) {
        return UIL_TAI_HANLO;
    } else {
        return UIL_ENGLISH;
    }
}

class ApplicationImpl : Application {
  public:
    ApplicationImpl(HMODULE hmod) :
        m_display(AppearanceProps(this)), m_input(InputProps(this)), m_about(PropSheet(this)) {
        g_module = hmod;
    }

    virtual UiLanguage uilang() override {
        if (uilang_set) {
            return g_config->appearance().ui_language();
        }

        return GetSystemLang();
    }

    virtual void set_uilang(UiLanguage lang) override {
        uilang_set = true;
        g_config->mutable_appearance()->set_ui_language(lang);
        m_display.Reload();
    }

    int ShowDialog() {
        LoadConfig();

        auto pages = std::vector<HPROPSHEETPAGE>();

        pages.push_back(m_display.psp(g_module, IDD_APPEARANCETAB, g_config));
        pages.push_back(m_input.psp(g_module, IDD_INPUTTAB, g_config));
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
        psh.pszCaption = L"起引設定 Khíín Siat-tēng";
        psh.pszIcon = MAKEINTRESOURCE(IDI_KHIINSETTINGS);

        return ::PropertySheet(&psh);
    }

  private:
    bool uilang_set = false;
    UiLanguage m_lang = UIL_TAI_HANLO;
    AppearanceProps m_display;
    InputProps m_input;
    PropSheet m_about;
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
    auto app = khiin::win32::settings::ApplicationImpl(hInstance);
    return app.ShowDialog();
}
