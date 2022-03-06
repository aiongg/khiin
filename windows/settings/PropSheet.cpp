#include "pch.h"

#include "PropSheet.h"

#include "Application.h"
#include "Strings.h"

namespace khiin::win32::settings {
namespace {

void SetTitle(HWND page_hwnd, uint32_t page_rid, UiLanguage lang) {
    auto title = Strings::T(page_rid, lang);
    if (!title.empty()) {
        TCITEM tci;
        tci.mask = TCIF_TEXT;
        tci.pszText = &title[0];
        auto parent = ::GetParent(page_hwnd);
        auto tab_hwnd = PropSheet_GetTabControl(parent);
        auto pageidx = PropSheet_HwndToIndex(page_hwnd, parent);
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

using namespace messages;

PropSheet::PropSheet(Application *app) : m_app(app) {}

LRESULT CALLBACK PropSheet::StaticDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    PropSheetPage *psp = NULL;
    PropSheet *self = NULL;

    if (uMsg == WM_INITDIALOG) {
        auto psp = reinterpret_cast<PropSheetPage *>(lParam);
        self = reinterpret_cast<PropSheet *>(psp->self);
        self->SetHwnd(hwnd);
        ::SetWindowLongPtr(hwnd, DWLP_USER, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<PropSheet *>(::GetWindowLongPtr(hwnd, DWLP_USER));
    }

    if (self) {
        return self->DlgProc(uMsg, wParam, lParam);
    } else {
        return FALSE;
    }
}

HPROPSHEETPAGE hpsp = NULL;

HPROPSHEETPAGE PropSheet::psp(HMODULE hmod, int template_id, messages::AppConfig *config) {
    m_module = hmod;
    m_template_id = template_id;
    m_config = config;

    m_psp.dwSize = sizeof(PropSheetPage);
    m_psp.hInstance = m_module;
    m_psp.pszTemplate = MAKEINTRESOURCE(m_template_id);
    m_psp.pfnDlgProc = &StaticDlgProc;
    m_psp.self = reinterpret_cast<intptr_t>(this);

    m_hpsp = ::CreatePropertySheetPage(&m_psp);
    return m_hpsp;
}

void PropSheet::SetHwnd(HWND hwnd) {
    m_hwnd = hwnd;
}

void PropSheet::InitComboBox(uint32_t control_rid, std::vector<uint32_t> const &option_rids, int selected_index) {
    HWND cb_hwnd = ::GetDlgItem(m_hwnd, control_rid);
    ComboBox_ResetContent(cb_hwnd);
    auto lang = m_app->uilang();
    for (auto rid : option_rids) {
        auto str = Strings::T(rid, lang);
        if (!str.empty()) {
            ComboBox_AddString(cb_hwnd, str.c_str());
        }
    }
    ComboBox_SetCurSel(cb_hwnd, selected_index);
}

INT_PTR PropSheet::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) {
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

void PropSheet::Reload() {
    Initialize();
}

void PropSheet::Initialize() {
    auto lang = m_app->uilang();

    SetTitle(m_hwnd, m_template_id, lang);

    for (auto rid : m_string_ids) {
        HWND hcontrol = ::GetDlgItem(m_hwnd, rid);
        if (hcontrol) {
            SetText(hcontrol, rid, lang);
        }
    }
}

void PropSheet::Cancel() {
    // do nothing
}
void PropSheet::Finalize() {
    // do nothing
}

void PropSheet::OnChange() {
    auto parent = ::GetParent(m_hwnd);
    auto apply_btn = ::GetDlgItem(parent, ID_APPLY_NOW);
    Button_Enable(apply_btn, TRUE);
}

} // namespace khiin::win32::settings
