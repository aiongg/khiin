#include "pch.h"

#include "resource.h"

#include "AppearanceProps.h"

#include "Application.h"
#include "Strings.h"

namespace khiin::win32::settings {
namespace {
using namespace messages;

constexpr int kSizeTrackbarMin = 0;
constexpr int kSizeTrackbarMax = 4;

auto kThemeNameStringRids = std::vector<uint32_t>{
    IDS_LIGHT_THEME,
    IDS_DARK_THEME,
};

auto kDisplayLanguageStringRids = std::vector<uint32_t>{
    IDS_DISPLAY_LANGUAGE_SYS,
    IDS_DISPLAY_LANGUAGE_EN,
    IDS_DISPLAY_LANGUAGE_HANLO,
    IDS_DISPLAY_LANGUAGE_LO,
};

auto kControlResIds = std::vector<uint32_t>{
    IDL_COLOR,
    IDL_CANDIDATE_SIZE,
    IDL_CANDIDATE_SIZE_S,
    IDL_CANDIDATE_SIZE_L,
    IDL_DISPLAY_LANGUAGE,
    // IDL_DISPLAY_LANGUAGE,
    IDL_EDIT_TRY,
};

} // namespace

AppearanceProps::AppearanceProps(Application *app) : PropSheet(app) {
    m_string_ids = kControlResIds;
}

void AppearanceProps::Initialize() {
    InitComboBox(IDC_COMBOBOX_THEME_COLOR, kThemeNameStringRids, m_config->appearance().colors());
    InitComboBox(IDC_DISPLAY_LANGUAGE, kDisplayLanguageStringRids, static_cast<int>(m_app->uilang()));

    HWND size_hwnd = ::GetDlgItem(m_hwnd, IDC_CANDIDATE_SIZE);
    ::SendMessage(size_hwnd, TBM_SETRANGE, TRUE, MAKELPARAM(kSizeTrackbarMin, kSizeTrackbarMax));
    ::SendMessage(size_hwnd, TBM_SETPOS, TRUE, m_config->appearance().size());

    PropSheet::Initialize();
}

void AppearanceProps::Finalize() {
    HWND theme_hwnd = ::GetDlgItem(m_hwnd, IDC_COMBOBOX_THEME_COLOR);
    HWND size_hwnd = ::GetDlgItem(m_hwnd, IDC_CANDIDATE_SIZE);

    auto appearance = m_config->mutable_appearance();
    appearance->set_colors(ComboBox_GetCurSel(theme_hwnd));
    appearance->set_size(::SendMessage(size_hwnd, TBM_GETPOS, 0, 0));

    HWND lang_hwnd = ::GetDlgItem(m_hwnd, IDC_DISPLAY_LANGUAGE);
    m_app->set_uilang(static_cast<UiLanguage>(ComboBox_GetCurSel(lang_hwnd)));
}

} // namespace khiin::win32::settings
