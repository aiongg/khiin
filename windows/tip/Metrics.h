#pragma once

namespace khiin::win32 {

enum class DisplaySize {
    S,
    M,
    L,
    XL,
    XXL,
};

struct Metrics {
    float padding = 0.0f;
    float padding_sm = 0.0f;
    float marker_w = 0.0f;
    float marker_h = 0.0f;
    float font_size = 0.0f;
    float font_size_sm = 0.0f;
    float row_height = 0.0f;
    uint32_t min_col_w_single = 0;
    uint32_t min_col_w_multi = 0;
    uint32_t qs_col_w = 0;
};

Metrics GetMetricsForSize(DisplaySize size);

} // namespace khiin::win32
