#include "pch.h"

#include "CandidateWindow.h"

#include <algorithm>

#include "Colors.h"
#include "Geometry.h"
#include "GridLayout.h"
#include "Metrics.h"
#include "RenderFactory.h"
#include "Utils.h"

namespace khiin::win32 {

namespace {

using namespace winrt;
using namespace D2D1;
using namespace messages;
using namespace geometry;

static inline auto divide_ceil(unsigned int x, unsigned int y) {
    return x / y + (x % y != 0);
}

static inline constexpr int kCornerRadius = 4;

static const DWORD kDwStyle = WS_BORDER | WS_POPUP;
static const DWORD kDwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;

float CenterTextLayoutY(IDWriteTextLayout *layout, float available_height) {
    DWRITE_TEXT_METRICS dwt_metrics;
    check_hresult(layout->GetMetrics(&dwt_metrics));
    return (available_height - dwt_metrics.height) / 2;
}

struct CandidateLayout {
    Candidate const *candidate = nullptr;
    com_ptr<IDWriteTextLayout> layout = com_ptr<IDWriteTextLayout>();
};

using CandidateLayoutGrid = GridLayoutContainer<CandidateLayout>;

class CandidateWindow2Impl : public CandidateWindow {
  public:
    void Create(HWND parent) {
        BaseWindow::Create(NULL, // clang-format off
            kDwStyle,
            kDwExStyle,
            0, 0, 100, 100,
            parent,
            NULL); // clang-format on
    }

    virtual std::wstring const &ClassName() const override {
        return kCandidateWindowClassName;
    }

    virtual void SetCandidates(DisplayMode display_mode, CandidateGrid *candidate_grid, int focused_id, size_t qs_col,
                               bool qs_active, RECT text_position) override {
        m_candidate_grid = candidate_grid;
        m_display_mode = display_mode;
        m_focused_id = focused_id;
        m_quickselect_col = qs_col;
        m_quickselect_active = qs_active;
        m_text_rect = text_position;
        CalculateLayout();
    }

    virtual void OnConfigChanged(AppConfig* config) override {
        GuiWindow::OnConfigChanged(config);
        m_metrics = GetMetricsForSize(static_cast<DisplaySize>(config->appearance().size()));
        DiscardGraphicsResources();
    }

    virtual void RegisterCandidateSelectListener(CandidateSelectListener *listener) override {
        if (std::find(m_focus_listeners.begin(), m_focus_listeners.end(), listener) == m_focus_listeners.end()) {
            m_focus_listeners.push_back(listener);
        }
    }

    virtual void OnMouseMove(Point pt) override {
        if (!ClientHitTest(pt)) {
            m_mouse_focused_id = -1;
            return;
        }
        ClientDp(pt);
        auto item = m_layout_grid.GetItemByHit(pt);
        auto hover_candidate_id = -1;
        if (item) {
            hover_candidate_id = item->candidate->id();
        }
        if (m_mouse_focused_id != hover_candidate_id) {
            m_mouse_focused_id = hover_candidate_id;
            ::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        }
    }

    virtual bool OnClick(Point pt) override {
        if (!ClientHitTest(pt)) {
            Hide();
            return false;
        }

        ClientDp(pt);
        auto item = m_layout_grid.GetItemByHit(pt);
        if (item) {
            NotifyCandidateSelectListeners(item->candidate);
        }
        return true;
    }

    void NotifyCandidateSelectListeners(Candidate const *candidate) {
        for (auto listener : m_focus_listeners) {
            if (listener) {
                listener->OnSelectCandidate(candidate->id());
            }
        }
    }

  private:
    void DiscardGraphicsResources() {
        m_target = nullptr;
        m_brush = nullptr;
        m_textformat = nullptr;
        m_textformat_sm = nullptr;
    }

    void EnsureTextFormat() {
        if (!m_textformat) {
            m_textformat = m_factory->CreateTextFormat("Arial", m_metrics.font_size);
        }

        if (!m_textformat_sm) {
            m_textformat_sm = m_factory->CreateTextFormat("Arial", m_metrics.font_size_sm);
        }
    }

    void EnsureBrush() {
        if (!m_brush) {
            check_hresult(m_target->CreateSolidColorBrush(m_colors.text, m_brush.put()));
        }
    }

    uint32_t MinColWidth() {
        return (m_display_mode == DisplayMode::Grid) ? m_metrics.min_col_w_multi : m_metrics.min_col_w_single;
    }

    void AddLayoutToGrid(int row, int col, Candidate const *candidate) {
        auto &value = Utils::Widen(candidate->value());
        auto layout = m_factory->CreateTextLayout(candidate->value(), m_textformat, m_max_width, m_max_height);

        DWRITE_TEXT_METRICS dwtm;
        check_hresult(layout->GetMetrics(&dwtm));

        m_layout_grid.EnsureColumnWidth(col, static_cast<int>(dwtm.width + m_metrics.qs_col_w));
        m_layout_grid.EnsureRowHeight(static_cast<int>(dwtm.height));
        m_layout_grid.AddItem(row, col, CandidateLayout{candidate, std::move(layout)});
    }

    void CalculateLayout() {
        KHIIN_TRACE("");
        if (!m_factory || !m_candidate_grid) {
            return;
        }

        EnsureTextFormat();

        int n_cols = static_cast<int>(m_candidate_grid->size());
        int n_rows = static_cast<int>(m_candidate_grid->at(0).size());

        m_layout_grid = CandidateLayoutGrid(n_rows, n_cols, MinColWidth());
        m_layout_grid.SetRowPadding(static_cast<int>(m_metrics.padding));

        for (auto col_idx = 0; col_idx < n_cols; ++col_idx) {
            auto &candidates = m_candidate_grid->at(col_idx);
            for (size_t row_idx = 0; row_idx < candidates.size(); ++row_idx) {
                auto candidate = candidates[row_idx];
                AddLayoutToGrid(static_cast<int>(row_idx), col_idx, candidate);
            }
        }

        m_metrics.row_height = static_cast<float>(m_layout_grid.row_height());
        auto grid_size = m_layout_grid.GetGridSize();
        auto left = m_text_rect.left - m_metrics.qs_col_w * m_scale;
        auto top = m_text_rect.bottom * 1.0f;
        auto width = grid_size.width * m_scale;
        auto height = grid_size.height * m_scale;

        if (left + width > m_max_width) {
            left = m_max_width - width;
        }
        if (top + height > m_max_height) {
            top = m_text_rect.top - height;
        }

        auto l = static_cast<int>(left);
        auto t = static_cast<int>(top);
        auto w = static_cast<int>(width);
        auto h = static_cast<int>(height);

        ::SetWindowPos(m_hwnd, NULL, l, t, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
        ::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }

    void DrawFocusedBackground(float left, float top, float col_width) {
        m_brush->SetColor(m_colors.bg_selected);
        m_target->FillRoundedRectangle(
            D2D1::RoundedRect(D2D1::RectF(left, top, col_width - m_metrics.padding, top + m_metrics.row_height),
                              m_metrics.padding_sm, m_metrics.padding_sm),
            m_brush.get());
    }

    void DrawFocusedMarker(float left, float top) {
        m_brush->SetColor(m_colors.accent);
        m_target->FillRoundedRectangle(
            D2D1::RoundedRect(D2D1::RectF(left + m_metrics.marker_w,
                                          top + (m_metrics.row_height - m_metrics.marker_h) / 2,
                                          left + m_metrics.marker_w * 2,
                                          top + (m_metrics.row_height - m_metrics.marker_h) / 2 + m_metrics.marker_h),
                              2.0f, 2.0f),
            m_brush.get());
    }

    void DrawFocused(float left, float top, float col_width) {
        DrawFocusedBackground(left, top, col_width);
        DrawFocusedMarker(left, top);
    }

    void DrawQuickSelect(std::string label, float left, float top) {
        auto layout = m_factory->CreateTextLayout(label, m_textformat_sm, m_metrics.qs_col_w, m_metrics.row_height);
        auto x = left + m_metrics.marker_w * 2 + m_metrics.padding;
        auto y = top + CenterTextLayoutY(layout.get(), m_metrics.row_height);
        if (m_quickselect_active) {
            m_brush->SetColor(m_colors.text);
        } else {
            m_brush->SetColor(m_colors.text_disabled);
        }
        m_target->DrawTextLayout(D2D1::Point2F(x, y), layout.get(), m_brush.get());
    }

    void DrawCandidate(CandidateLayout &candidate, float left, float top) {
        auto x = left + m_metrics.qs_col_w;
        auto y = top + CenterTextLayoutY(candidate.layout.get(), m_metrics.row_height);
        m_brush->SetColor(m_colors.text);
        m_target->DrawTextLayout(D2D1::Point2F(x, y), candidate.layout.get(), m_brush.get(),
                                 D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
    }

    void Draw() {
        KHIIN_TRACE("");
        m_target->Clear(m_colors.bg);
        EnsureBrush();

        auto qs_label = 1;
        for (auto col_idx = 0; col_idx < m_layout_grid.cols(); ++col_idx) {
            auto &col = m_layout_grid.items[col_idx];
            for (auto row_idx = 0; static_cast<size_t>(row_idx) < col.size(); ++row_idx) {
                auto &value = m_layout_grid.GetItem(row_idx, col_idx);

                if (!value.candidate || !value.layout) {
                    continue;
                }

                auto cell = m_layout_grid.GetCellRect(row_idx, col_idx);
                auto left = cell.leftf();
                auto top = cell.topf();
                auto right = cell.rightf();
                auto bottom = cell.bottomf();
                auto width = cell.widthf();

                // Note: Uncomment to draw boxes around the candidates,
                // useful for debugging
                // auto rct = D2D1::RectF(left, top, right, bottom);
                // m_target->DrawRectangle(rct, m_brush.get());

                if (value.candidate->id() == m_focused_id) {
                    DrawFocused(left, top, width);
                } else if (value.candidate->id() == m_mouse_focused_id) {
                    DrawFocusedBackground(left, top, width);
                }

                if (col_idx == m_quickselect_col) {
                    DrawQuickSelect(std::to_string(qs_label), left, top);
                    ++qs_label;
                }

                DrawCandidate(value, left, top);
            }
        }
    }

    virtual void Render() override {
        KHIIN_TRACE("");
        try {
            EnsureRenderTarget();

            PAINTSTRUCT ps;
            RECT rc;
            ::GetClientRect(m_hwnd, &rc);
            ::BeginPaint(m_hwnd, &ps);

            m_target->BindDC(ps.hdc, &rc);
            m_target->BeginDraw();
            Draw();
            auto hr = m_target->EndDraw();
            if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
                DiscardGraphicsResources();
            }
            ::EndPaint(m_hwnd, &ps);
        } catch (...) {
            DiscardGraphicsResources();
        }
    }

    com_ptr<ID2D1SolidColorBrush> m_brush = nullptr;
    com_ptr<IDWriteTextFormat> m_textformat = nullptr;
    com_ptr<IDWriteTextFormat> m_textformat_sm = nullptr;

    CandidateLayoutGrid m_layout_grid;
    CandidateGrid *m_candidate_grid = nullptr;
    RECT m_text_rect = {};
    Metrics m_metrics = GetMetricsForSize(DisplaySize::S);
    DisplayMode m_display_mode = DisplayMode::ShortColumn;
    int m_focused_id = -1;
    size_t m_quickselect_col = 0;
    bool m_quickselect_active = false;
    int m_mouse_focused_id = -1;
    bool m_tracking_mouse = false;

    std::vector<CandidateSelectListener *> m_focus_listeners;
};

} // namespace

const std::wstring kCandidateWindowClassName = L"CandidateWindow";

GUID kCandidateWindowGuid // 829893fa-728d-11ec-8c6e-e0d46491b35a
    = {0x829893fa, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

CandidateWindow *CandidateWindow::Create(HWND parent) {
    auto impl = new CandidateWindow2Impl();
    impl->Create(parent);
    return impl;
}

} // namespace khiin::win32
