#include "pch.h"

#include "Metrics.h"

namespace khiin::win32 {
namespace {

} // namespace

Metrics GetMetricsForSize(DisplaySize size) {
    auto metrics = Metrics();
    metrics.marker_w = 4.0f;
    metrics.font_size_sm = 16.0f;
    metrics.min_col_w_multi = 80;
    metrics.min_col_w_single = 160;
    metrics.qs_col_w = 44;

    switch (size) {
    case DisplaySize::S:
        metrics.padding = 8.0f;
        metrics.padding_sm = 4.0f;
        metrics.font_size = 16.0f;
        metrics.marker_h = 16.0f;
        break;
    case DisplaySize::M:
        metrics.padding = 10.0f;
        metrics.padding_sm = 5.0f;
        metrics.font_size = 20.0f;
        metrics.marker_h = 20.0f;
        break;
    case DisplaySize::L:
        metrics.padding = 12.0f;
        metrics.padding_sm = 6.0f;
        metrics.font_size = 24.0f;
        metrics.marker_h = 24.0f;
        break;
    case DisplaySize::XL:
        metrics.padding = 14.0f;
        metrics.padding_sm = 7.0f;
        metrics.font_size = 28.0f;
        metrics.marker_h = 24.0f;
        break;
    case DisplaySize::XXL:
        metrics.padding = 16.0f;
        metrics.padding_sm = 8.0f;
        metrics.font_size = 32.0f;
        metrics.marker_h = 24.0f;
        break;
    }

    metrics.row_height = metrics.font_size + metrics.padding;

    return metrics;
}

} // namespace khiin::win32