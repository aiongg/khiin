#include "pch.h"

#include "AppearanceProps.h"

#include "proto/proto.h"
#include "tip/Config.h"

#include "Application.h"
#include "Strings.h"
#include "resource.h"

namespace khiin::win32::settings {
namespace {
using namespace proto;

constexpr int kSizeTrackbarMin = 0;
constexpr int kSizeTrackbarMax = 10;

auto kThemeNameStringRids = std::vector<uint32_t>{
    IDS_LIGHT_THEME,
    IDS_DARK_THEME,
};

auto kDisplayLanguageStringRids = std::vector<uint32_t>{
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

AppearanceProps::AppearanceProps(Application *app) : PropSheetPage(app) {
    m_string_ids = kControlResIds;
}

void AppearanceProps::Initialize() {

    InitComboBox(IDC_COMBOBOX_THEME_COLOR, kThemeNameStringRids, static_cast<int>(Config::GetUiColors()));
    InitComboBox(IDC_DISPLAY_LANGUAGE, kDisplayLanguageStringRids, static_cast<int>(Config::GetUiLanguage()));

    auto item = ::GetDlgItem(m_hwnd, IDC_CANDIDATE_SIZE);
    ::SendMessage(item, TBM_SETRANGE, TRUE, MAKELPARAM(kSizeTrackbarMin, kSizeTrackbarMax));
    ::SendMessage(item, TBM_SETPOS, TRUE, Config::GetUiSize());
    //::SendMessage(size_hwnd, TBM_SETTICFREQ, 10, 0);

    PropSheetPage::Initialize();
}

void AppearanceProps::Finalize() {
    Config::SetUiColors(static_cast<UiColors>(ComboBox_GetCurSel(::GetDlgItem(m_hwnd, IDC_COMBOBOX_THEME_COLOR))));
    Config::SetUiSize(static_cast<int>(::SendMessage(::GetDlgItem(m_hwnd, IDC_CANDIDATE_SIZE), TBM_GETPOS, 0, 0)));
    Config::SetUiLanguage(static_cast<UiLanguage>(ComboBox_GetCurSel(::GetDlgItem(m_hwnd, IDC_DISPLAY_LANGUAGE))));
}

} // namespace khiin::win32::settings
