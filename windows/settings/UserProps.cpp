#include "pch.h"

#include "UserProps.h"

#include "tip/Config.h"

#include "Strings.h"

namespace khiin::win32::settings {
namespace {

auto kControlIds = std::vector<uint32_t>{
    IDL_RESET_USER_DATA,
    IDC_RESET_BTN,
};

} // namespace

UserProps::UserProps(Application *app) : PropSheetPage(app) {
    m_string_ids = kControlIds;
}

void UserProps::Initialize() {
    PropSheetPage::Initialize();
}

void UserProps::Finalize() {
    PropSheetPage::Finalize();
}

INT_PTR UserProps::DlgProc(UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_COMMAND:
        if (LOWORD(wparam) == IDC_RESET_BTN) {
            HandleClearUserData();
            return 0;
        }
        break;
    }

    return PropSheetPage::DlgProc(msg, wparam, lparam);
}

void UserProps::HandleClearUserData() {
    Config::ClearUserHistory();
    auto hbtn = ItemById(IDC_RESET_BTN);
    BOOL enable = FALSE;
    ::Button_Enable(hbtn, enable);
    auto label = Strings::T(IDL_RESET_BUTTON_DONE, Config::GetUiLanguage());
    ::Button_SetText(hbtn, label.c_str());
}

} // namespace khiin::win32::settings