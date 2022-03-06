#include "pch.h"

#include "resource.h"

#include "AppearanceProps.h"

#include <KhiinPJH/Colors.h>

#include "KhiinSettings.h"

namespace khiin::win32::settings {
namespace {
using namespace messages;

auto kThemeList = Colors::ColorSchemeNames();
constexpr int kSizeTrackbarMin = 0;
constexpr int kSizeTrackbarMax = 4;

// clang-format off
auto kControlResIds = std::vector<uint32_t>{
    IDL_COLOR,
    IDL_CANDIDATE_SIZE,
    IDL_CANDIDATE_SIZE_S,
    IDL_CANDIDATE_SIZE_L,
    IDL_EDIT_TRY,
};

auto kEnStringResIds = std::vector<uint32_t>{
    IDL_COLOR_EN,
    IDL_CANDIDATE_SIZE_EN,
    IDL_CANDIDATE_SIZE_S_EN,
    IDL_CANDIDATE_SIZE_L_EN,
    IDL_EDIT_TRY_EN,
};

auto kHlStringResIds = std::vector<uint32_t>{
    IDL_COLOR_HL,
    IDL_CANDIDATE_SIZE_HL,
    IDL_CANDIDATE_SIZE_S_HL,
    IDL_CANDIDATE_SIZE_L_HL,
    IDL_EDIT_TRY_HL,
};

// clang-format on

} // namespace

AppearanceProps::AppearanceProps(KhiinSettings *app) : PropSheet(app) {
    m_res_ids = kControlResIds;
    m_translations.insert(std::make_pair(UiLanguage::EN, kEnStringResIds));
    m_translations.insert(std::make_pair(UiLanguage::HL, kHlStringResIds));
}

void AppearanceProps::Initialize() {
    HWND theme_hwnd = ::GetDlgItem(m_hwnd, IDC_COMBOBOX_THEME_COLOR);
    for (auto &str : kThemeList) {
        auto wstr = Utils::Widen(str);
        ComboBox_AddString(theme_hwnd, &wstr[0]);
    }
    ComboBox_SetCurSel(theme_hwnd, m_config->appearance().colors());

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
}

} // namespace khiin::win32::settings
