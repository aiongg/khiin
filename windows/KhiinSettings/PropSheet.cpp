#include "pch.h"

#include "PropSheet.h"

#include "KhiinSettings.h"

namespace khiin::win32::settings {
namespace {

std::wstring GetString(uint32_t rid) {
    const wchar_t *buf = nullptr;
    int len = ::LoadString(nullptr, rid, reinterpret_cast<LPWSTR>(&buf), 0);
    if (len > 0) {
        return std::wstring{buf, static_cast<size_t>(len)};
    }
    return std::wstring{};
}

} // namespace

using namespace messages;

PropSheet::PropSheet(KhiinSettings *app) : m_app(app) {}

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

void PropSheet::_T(uint32_t control_rid, uint32_t string_rid) {
    HWND hwnd_item = ::GetDlgItem(m_hwnd, control_rid);
    if (hwnd_item) {
        ::SetWindowText(hwnd_item, GetString(string_rid).c_str());
    }
}

void PropSheet::Initialize() {
    auto lang = m_app->uilang();

    auto it = m_translations.find(lang);
    auto &strs = it == m_translations.end() ? m_translations.at(UiLanguage::HL) : it->second;

    for (auto i = 0; i < m_res_ids.size(); ++i) {
        _T(m_res_ids.at(i), strs.at(i));
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
