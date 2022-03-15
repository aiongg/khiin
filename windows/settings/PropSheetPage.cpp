#include "pch.h"

#include "PropSheetPage.h"

#include "proto/proto.h"
#include "tip/Config.h"

#include "Application.h"
#include "Strings.h"

namespace khiin::win32::settings {
namespace {
using namespace proto;

void SetTitle(HWND page_hwnd, uint32_t page_rid, UiLanguage lang) {
    auto title = Strings::T(page_rid, lang);
    if (!title.empty()) {
        TCITEM tci;
        tci.mask = TCIF_TEXT;
        tci.pszText = &title[0];
        auto parent = ::GetParent(page_hwnd);
        auto pageidx = PropSheet_HwndToIndex(parent, page_hwnd);
        auto tab_hwnd = PropSheet_GetTabControl(parent);
        TabCtrl_SetItem(tab_hwnd, pageidx, &tci);
    }
}

void SetText(HWND hwnd, uint32_t rid, UiLanguage lang) {
    auto str = Strings::T(rid, lang);
    if (!str.empty()) {
        ::SetWindowText(hwnd, str.c_str());
    }
}

} // namespace

using namespace proto;

PropSheetPage::PropSheetPage(Application *app) : m_app(app) {}

LRESULT CALLBACK PropSheetPage::StaticDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    PSP *psp = NULL;
    PropSheetPage *self = NULL;

    if (uMsg == WM_INITDIALOG) {
        auto psp = reinterpret_cast<PSP *>(lParam);
        self = reinterpret_cast<PropSheetPage *>(psp->self);
        self->SetHwnd(hwnd);
        ::SetWindowLongPtr(hwnd, DWLP_USER, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<PropSheetPage *>(::GetWindowLongPtr(hwnd, DWLP_USER));
    }

    if (self) {
        return self->DlgProc(uMsg, wParam, lParam);
    } else {
        return FALSE;
    }
}

HPROPSHEETPAGE PropSheetPage::psp(HMODULE hmod, int template_id, AppConfig *config) {
    m_module = hmod;
    m_template_id = template_id;
    m_config = config;
    m_tab_label = Strings::T(m_template_id, Config::GetUiLanguage());

    m_psp.dwFlags = PSP_USETITLE;
    m_psp.pszTitle = &m_tab_label[0];
    m_psp.dwSize = sizeof(PropSheetPage);
    m_psp.hInstance = m_module;
    m_psp.pszTemplate = MAKEINTRESOURCE(m_template_id);
    m_psp.pfnDlgProc = &StaticDlgProc;
    m_psp.self = reinterpret_cast<intptr_t>(this);

    m_hpsp = ::CreatePropertySheetPage(&m_psp);
    return m_hpsp;
}

HWND PropSheetPage::hwnd() {
    return m_hwnd;
}

HWND PropSheetPage::ItemById(uint32_t res_id) {
    return ::GetDlgItem(m_hwnd, res_id);
}

void PropSheetPage::SetHwnd(HWND hwnd) {
    m_hwnd = hwnd;
}

void PropSheetPage::InitComboBox(uint32_t control_rid, std::vector<uint32_t> const &option_rids, int selected_index) {
    HWND cb_hwnd = ::GetDlgItem(m_hwnd, control_rid);
    ComboBox_ResetContent(cb_hwnd);
    auto lang = Config::GetUiLanguage();
    for (auto rid : option_rids) {
        auto str = Strings::T(rid, lang);
        if (!str.empty()) {
            ComboBox_AddString(cb_hwnd, str.c_str());
        }
    }
    ComboBox_SetCurSel(cb_hwnd, selected_index);
}

INT_PTR PropSheetPage::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        Initialize();
        break;
    case WM_NOTIFY:
        switch (LPNMHDR(lParam)->code) {
        case PSN_KILLACTIVE:
            ::SetWindowLong(m_hwnd, DWLP_MSGRESULT, FALSE);
            break;
        case PSN_APPLY:
            Finalize();
            break;
        case PSN_QUERYCANCEL:
            Cancel();
            break;
        }
        break;
    case WM_COMMAND:
        switch (HIWORD(wParam)) {
        case BN_CLICKED: {
            auto btn = LOWORD(wParam);
            auto x = 5;
            [[fallthrough]];
        }
        case EN_CHANGE:
            [[fallthrough]];
        case CBN_SELCHANGE:
            OnChange();
        }
        break;
    case WM_HSCROLL:
        OnChange();
        break;
    }

    return FALSE;
}

void PropSheetPage::Reload() {
    Initialize();
    PropSheet_UnChanged(::GetParent(m_hwnd), m_hwnd);
}

void PropSheetPage::Initialize() {
    auto lang = Config::GetUiLanguage();

    SetTitle(m_hwnd, m_template_id, lang);

    for (auto rid : m_string_ids) {
        HWND hcontrol = ::GetDlgItem(m_hwnd, rid);
        if (hcontrol) {
            SetText(hcontrol, rid, lang);
        }
    }
}

void PropSheetPage::Cancel() {
    // do nothing
}
void PropSheetPage::Finalize() {
    // do nothing
}

void PropSheetPage::OnChange() {
    auto parent = ::GetParent(m_hwnd);
    auto apply_btn = ::GetDlgItem(parent, ID_APPLY_NOW);
    Button_Enable(apply_btn, TRUE);
}

} // namespace khiin::win32::settings
