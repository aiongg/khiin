#include "pch.h"

#include "AppearanceProps.h"

#include <KhiinPJH/Colors.h>

namespace khiin::win32::settings {
namespace {
using namespace messages;

auto kThemeList = Colors::ColorSchemeNames();

size_t ThemeIndex(std::string const &theme) {
    if (auto it = std::find(kThemeList.begin(), kThemeList.end(), theme); it != kThemeList.end()) {
        return std::distance(kThemeList.begin(), it);
    }
    return 0;
}

} // namespace

AppearanceProps::AppearanceProps(AppConfig *config, HINSTANCE instance, int template_id) :
    PropSheet(instance, template_id), config(config) {}

void AppearanceProps::Initialize() {
    HWND theme_hwnd = ::GetDlgItem(m_hwnd, IDC_COMBOBOX_THEME_COLOR);

    for (auto &str : kThemeList) {
        auto wstr = Utils::Widen(str);
        ComboBox_AddString(theme_hwnd, &wstr[0]);
    }
    ComboBox_SetCurSel(theme_hwnd, ThemeIndex(config->appearance().theme()));
}

void AppearanceProps::Finalize() {
    HWND theme_hwnd = ::GetDlgItem(m_hwnd, IDC_COMBOBOX_THEME_COLOR);

    auto index = ComboBox_GetCurSel(theme_hwnd);
    config->mutable_appearance()->set_theme(kThemeList.at(index));
}

} // namespace khiin::win32::settings
