#include "pch.h"

#include "Colors.h"

#include "proto/proto.h"

namespace khiin::win32 {
namespace {
using namespace D2D1;
using namespace khiin::proto;

ColorScheme const kLightScheme = ColorScheme{
    ColorF(ColorF::Black),          // text
    ColorF(ColorF::Gray),           // text disabled
    ColorF(ColorF::DarkGray),       // text extended
    ColorF(ColorF::ForestGreen),    // text hint
    ColorF(0.98f, 0.98f, 0.98f),    // background
    ColorF(0.92f, 0.92f, 0.92f),    // background selected
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

const std::vector<ColorScheme> kColorSchemeMap = {kLightScheme, kDarkScheme};

} // namespace

ColorScheme const &Colors::GetScheme(AppConfig *config) {
    if (config->has_appearance()) {
        auto colors = config->appearance().colors();
    
        if (colors < static_cast<int32_t>(kColorSchemeMap.size())) {
            return kColorSchemeMap.at(colors);
        }
    }

    return kLightScheme;
}

} // namespace khiin::win32
