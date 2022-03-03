#include "pch.h"

#include "Colors.h"

namespace khiin::win32 {
namespace {
using namespace D2D1;

const std::string kLightTheme = u8"天光 Thiⁿ-kng";
const std::string kDarkTheme = u8"烏暗 O͘-àm";

ColorScheme const kLightScheme = ColorScheme{
    ColorF(ColorF::Black),          // text
    ColorF(ColorF::Gray),           // text disabled
    ColorF(ColorF::DarkGray),       // text extended
    ColorF(ColorF::ForestGreen),    // text hint
    ColorF(0.97f, 0.97f, 0.97f),    // background
    ColorF(0.90f, 0.90f, 0.90f),    // background selected
    ColorF(ColorF::CornflowerBlue), // accent
};

ColorScheme const kDarkScheme = ColorScheme{
    ColorF(0.98f, 0.98f, 0.98f),          // text
    ColorF(ColorF::LightGray),            // text disabled
    ColorF(ColorF::WhiteSmoke),           // text extended
    ColorF(ColorF::LightGoldenrodYellow), // text hint
    ColorF(0.17f, 0.17f, 0.17f),          // background
    ColorF(0.12f, 0.12f, 0.12f),          // background selected
    ColorF(ColorF::LightSkyBlue),         // accent
};

const std::vector<std::string> kThemeNames = {kLightTheme, kDarkTheme};
const std::vector<ColorScheme> kColorSchemeMap = {kLightScheme, kDarkScheme};

} // namespace

std::vector<std::string> const &Colors::ColorSchemeNames() {
    return kThemeNames;
}

ColorScheme const &Colors::GetScheme(messages::AppConfig *config) {
    auto colors = config->appearance().colors();
    
    if (colors < kColorSchemeMap.size()) {
        return kColorSchemeMap.at(colors);
    }

    return kLightScheme;
}

} // namespace khiin::win32
