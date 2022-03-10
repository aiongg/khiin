#include "pch.h"

#include "Metrics.h"

namespace khiin::win32 {
namespace {

constexpr auto kScaleMax = 10;

struct MinMax {
    float min = 0.0f;
    float max = 0.0f;
};

constexpr auto kFontSize = MinMax{16.0f, 32.0f};
constexpr auto kFontSizeSm = MinMax{16.0f, 20.0f};
constexpr auto kPadding = MinMax{8.0f, 12.0f};
constexpr auto kPaddingSm = MinMax{4.0f, 6.0f};
constexpr auto kMarkerHeight = MinMax{16.0f, 24.0f};

constexpr float ScaleF(MinMax range, int scale) {
    auto s = scale / static_cast<float>(kScaleMax);
    return s * (range.max - range.min) + range.min;
}

} // namespace

Metrics GetMetricsScaled(int scale) {
    if (scale < 0) {
        scale = 0;
    } else if (scale > kScaleMax) {
        scale = kScaleMax;
    }

    auto metrics = Metrics();
    metrics.marker_w = 4.0f;
    metrics.min_col_w_multi = 80;
    metrics.min_col_w_single = 160;
    metrics.qs_col_w = 44;

    metrics.padding = ScaleF(kPadding, scale);
    metrics.padding_sm = ScaleF(kPaddingSm, scale);
    metrics.font_size = ScaleF(kFontSize, scale);
    metrics.font_size_sm = ScaleF(kFontSizeSm, scale);
    metrics.marker_h = ScaleF(kMarkerHeight, scale);

    metrics.row_height = metrics.font_size + metrics.padding;

    return metrics;
}

} // namespace khiin::win32