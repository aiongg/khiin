#pragma once

#include "pch.h"

namespace khiin::proto {
class AppConfig;
} // namespace khiin::proto

namespace khiin::win32 {

struct ColorScheme {
    D2D1::ColorF text = D2D1::ColorF(0, 0, 0);
    D2D1::ColorF text_disabled = D2D1::ColorF(0, 0, 0);
    D2D1::ColorF text_extended = D2D1::ColorF(0, 0, 0);
    D2D1::ColorF text_hint = D2D1::ColorF(0, 0, 0);
    D2D1::ColorF bg = D2D1::ColorF(1, 1, 1);
    D2D1::ColorF bg_selected = D2D1::ColorF(1, 1, 1);
    D2D1::ColorF accent = D2D1::ColorF(0, 0, 1);
};

class Colors {
  public:
    static std::vector<std::string> const &ColorSchemeNames();
    static ColorScheme const &GetScheme(proto::AppConfig *config);
};

} // namespace khiin::win32
