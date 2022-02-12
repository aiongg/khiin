#pragma once

#include "pch.h"

namespace khiin::win32 {

inline const D2D1::ColorF kColorText = D2D1::ColorF(D2D1::ColorF::Black);
inline const D2D1::ColorF kColorTextDisabled = D2D1::ColorF(D2D1::ColorF::Gray);
inline const D2D1::ColorF kColorTextExtended = D2D1::ColorF(D2D1::ColorF::DarkGray);
inline const D2D1::ColorF kColorTextHint = D2D1::ColorF(D2D1::ColorF::ForestGreen);
inline const D2D1::ColorF kColorBackground = D2D1::ColorF(0.97f, 0.97f, 0.97f);
inline const D2D1::ColorF kColorBackgroundSelected = D2D1::ColorF(0.90f, 0.90f, 0.90f);
inline const D2D1::ColorF kColorAccent = D2D1::ColorF(D2D1::ColorF::CornflowerBlue);

inline const D2D1::ColorF kDarkColorText = D2D1::ColorF(0.98f, 0.98f, 0.98f);
inline const D2D1::ColorF kDarkColorTextDisabled = D2D1::ColorF(D2D1::ColorF::LightGray);
inline const D2D1::ColorF kDarkColorTextExtended = D2D1::ColorF(D2D1::ColorF::WhiteSmoke);
inline const D2D1::ColorF kDarkColorTextHint = D2D1::ColorF(D2D1::ColorF::LightGoldenrodYellow);
inline const D2D1::ColorF kDarkColorBackground = D2D1::ColorF(0.17f, 0.17f, 0.17f);
inline const D2D1::ColorF kDarkColorBackgroundSelected = D2D1::ColorF(0.12f, 0.12f, 0.12f);
inline const D2D1::ColorF kDarkColorAccent = D2D1::ColorF(D2D1::ColorF::LightSkyBlue);

struct Colors {
    D2D1::ColorF text = kColorText;
    D2D1::ColorF text_disabled = kColorTextDisabled;
    D2D1::ColorF text_extended = kColorTextExtended;
    D2D1::ColorF text_hint = kColorTextHint;
    D2D1::ColorF bg = kColorBackground;
    D2D1::ColorF bg_selected = kColorBackgroundSelected;
    D2D1::ColorF accent = kColorAccent;
};

inline Colors const kLightColorScheme = Colors{};
inline Colors const kDarkColorScheme =
    Colors{// clang-format off
        kDarkColorText,
        kDarkColorTextDisabled,    
        kDarkColorTextExtended,
        kDarkColorTextHint,
        kDarkColorBackground,
        kDarkColorBackgroundSelected,
        kDarkColorAccent
    }; // clang-format on

} // namespace khiin::win32