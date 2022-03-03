#include "pch.h"

#include "AppearanceProps.h"

#include <KhiinPJH/Colors.h>

namespace khiin::win32::settings {
namespace {
using namespace messages;

auto kThemeList = Colors::ColorSchemeNames();
constexpr int kSizeTrackbarMin = 0;
constexpr int kSizeTrackbarMax = 4;

} // namespace

void AppearanceProps::Initialize() {
    HWND theme_hwnd = ::GetDlgItem(m_hwnd, IDC_COMBOBOX_THEME_COLOR);
    for (auto &str : kThemeList) {
        auto wstr = Utils::Widen(str);
        ComboBox_AddString(theme_hwnd, &wstr[0]);
    }
    ComboBox_SetCurSel(theme_hwnd, m_config->appearance().colors());

    HWND size_hwnd = ::GetDlgItem(m_hwnd, IDC_SIZE_SLIDER);
    ::SendMessage(size_hwnd, TBM_SETRANGE, TRUE, MAKELPARAM(kSizeTrackbarMin, kSizeTrackbarMax));
    ::SendMessage(size_hwnd, TBM_SETPOS, TRUE, m_config->appearance().size());
}

void AppearanceProps::Finalize() {
    HWND theme_hwnd = ::GetDlgItem(m_hwnd, IDC_COMBOBOX_THEME_COLOR);
    HWND size_hwnd = ::GetDlgItem(m_hwnd, IDC_SIZE_SLIDER);

    auto appearance = m_config->mutable_appearance();
    appearance->set_colors(ComboBox_GetCurSel(theme_hwnd));
    appearance->set_size(::SendMessage(size_hwnd, TBM_GETPOS, 0, 0));
}

} // namespace khiin::win32::settings
