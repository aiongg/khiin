#include "pch.h"

#include "InputProps.h"

#include "proto/proto.h"
#include "tip/Config.h"

#include "Application.h"
#include "resource.h"

namespace khiin::win32::settings {
namespace {

auto kControlResIds = std::vector<uint32_t>{
    // clang-format off
    IDL_INPUTMODE,
    IDC_INPUTMODE_CONTINUOUS,
    IDC_INPUTMODE_BASIC,
    IDC_INPUTMODE_PRO,
    IDC_OPTION_INPUTMODE_HOTKEY,
    IDC_OPTION_ALPHA_HOTKEY,
    IDC_OPTION_TELEX,
    IDC_OPTION_DOTTED_KHIN,
    IDC_OPTION_AUTOKHIN,
    IDC_OPTION_EASY_CH,
    IDL_DEFAULT_PUNCUTATION,
    IDC_PUNCT_FULL_WIDTH,
    IDC_PUNCT_HALF_WIDTH,
    // clang-format on
};

}

InputProps::InputProps(Application *app) : PropSheetPage(app) {
    m_string_ids = kControlResIds;
}

void InputProps::Initialize() {
    if (m_config->has_dotted_khin()) {
        auto hwnd = ::GetDlgItem(m_hwnd, IDC_OPTION_DOTTED_KHIN);
        Button_SetCheck(hwnd, m_config->dotted_khin().value() ? BST_CHECKED : BST_UNCHECKED);
    }
    m_config->dotted_khin();
    PropSheetPage::Initialize();
}

void InputProps::Finalize() {
    HWND item = NULL;

    item = ::GetDlgItem(m_hwnd, IDC_OPTION_DOTTED_KHIN);
    m_config->mutable_dotted_khin()->set_value(Button_GetCheck(item) == BST_CHECKED);
}

} // namespace khiin::win32::settings
