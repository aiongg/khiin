#include "pch.h"

#include "CandidateWindow.h"

#include <algorithm>

#include "Colors.h"
#include "Graphics.h"
#include "GridLayout.h"
#include "Utils.h"

namespace khiin::win32 {

namespace {

using namespace winrt;
using namespace D2D1;
using namespace messages;
using namespace geometry;

constexpr float kPadding = 8.0f;
constexpr float kPaddingSmall = 4.0f;
constexpr float kMarkerWidth = 4.0f;
constexpr float kMarkerHeight = 16.0f;
constexpr float kFontSize = 16.0f;
constexpr float kFontSizeSmall = 16.0f;
constexpr float kRowHeight = kFontSize + kPadding * 2;
constexpr uint32_t min_col_width_single = 160;
constexpr uint32_t min_col_width_expanded = 80;
constexpr uint32_t kQsColWidth = static_cast<uint32_t>(kPaddingSmall * 2 + kMarkerWidth + kPadding * 2 + kFontSize);

struct CWndMetrics {
    float padding = kPadding;
    float padding_sm = kPaddingSmall;
    float marker_w = kMarkerWidth;
    float marker_h = kMarkerHeight;
    float font_size = kFontSize;
    float font_size_sm = kFontSizeSmall;
    float row_height = kRowHeight;
    uint32_t min_col_w_single = 160;
    uint32_t min_col_w_multi = 80;
    uint32_t qs_col_w = kQsColWidth;
};

CWndMetrics GetMetricsForSize(DisplaySize size) {
    auto metrics = CWndMetrics();
    switch (size) {
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
    return metrics;
}

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

class CandidateWindowImpl : public CandidateWindow {
  public:
    void Create(HWND parent) {
        m_hwnd_parent = parent;

        Create_(NULL, // clang-format off
            kDwStyle,
            kDwExStyle,
            0, 0, 100, 100,
            m_hwnd_parent,
            NULL); // clang-format on
    }

    virtual LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override {
        switch (uMsg) {
        case WM_NCCREATE:
            KHIIN_TRACE("WM_NCCREATE");
#pragma warning(push)
#pragma warning(disable : 26812)
            ::DwmSetWindowAttribute(m_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &m_border_radius,
                                    sizeof(DWM_WINDOW_CORNER_PREFERENCE));
#pragma warning(pop)
            break;
        case WM_CREATE:
            KHIIN_TRACE("WM_CREATE");
            try {
                OnCreate();
                return 0;
            } catch (...) {
                return -1;
            }
        case WM_DISPLAYCHANGE:
            KHIIN_TRACE("WM_DISPLAYCHANGE");
            OnMonitorSizeChange();
            break;
        case WM_DPICHANGED:
            KHIIN_TRACE("WM_DPICHANGED");
            OnDpiChanged(HIWORD(wParam), (RECT *)lParam);
            return 0;
        case WM_MOUSEACTIVATE:
            KHIIN_TRACE("WM_MOUSEACTIVATE");
            break;
        case WM_MOUSEMOVE:
            OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            break;
        case WM_MOUSELEAVE:
            KHIIN_TRACE("WM_MOUSELEAVE");
            OnMouseLeave();
            break;
        case WM_LBUTTONDOWN:
            if (HandleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) {
                return 0;
            }
            break;
        case WM_PAINT:
            KHIIN_TRACE("WM_PAINT");
            Render();
            return 0;
        case WM_SIZE:
            KHIIN_TRACE("WM_SIZE");
            OnResize(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_WINDOWPOSCHANGING:
            KHIIN_TRACE("WM_WINDOWPOSCHANGING");
            OnMonitorSizeChange();
            break;
        case WM_NCCALCSIZE:
            KHIIN_TRACE("WM_NCCALCSIZE");
            break;
        case WM_NCPAINT:
            KHIIN_TRACE("WM_NCPAINT");
            break;
        case WM_NCACTIVATE:
            KHIIN_TRACE("WM_NCACTIVATE");
            break;
        case WM_ACTIVATE:
            KHIIN_TRACE("WM_ACTIVATE");
            break;
        case WM_DESTROY:
            KHIIN_TRACE("WM_DESTROY");
            break;
        case WM_NCDESTROY:
            KHIIN_TRACE("WM_NCDESTROY");
            break;
        case WM_MOVE:
            KHIIN_TRACE("WM_MOVE");
            break;
        case WM_WINDOWPOSCHANGED:
            KHIIN_TRACE("WM_WINDOWPOSCHANGED");
            break;
        case WM_SHOWWINDOW:
            KHIIN_TRACE("WM_SHOWWINDOW");
            break;
        case WM_ERASEBKGND:
            KHIIN_TRACE("WM_ERASEBKGND");
            break;
        default:
            KHIIN_TRACE("Unknown WM ({})", uMsg);
            break;
        }

        return ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }

    virtual std::wstring &class_name() const override {
        return kCandidateWindowClassName;
    }

    virtual void RegisterCandidateSelectListener(CandidateSelectListener *listener) override {
        if (std::find(m_focus_listeners.begin(), m_focus_listeners.end(), listener) == m_focus_listeners.end()) {
            m_focus_listeners.push_back(listener);
        }
    }

    virtual void Show() override {
        if (m_showing) {
            return;
        }

        ::ShowWindow(m_hwnd, SW_SHOWNA);
        m_showing = true;

        if (!m_tracking_mouse) {
            ::SetCapture(m_hwnd);
            m_tracking_mouse = true;
        }
    }

    virtual void Hide() override {
        if (!m_showing) {
            return;
        }

        ::ShowWindow(m_hwnd, SW_HIDE);
        m_showing = false;

        if (m_tracking_mouse) {
            ::ReleaseCapture();
            m_tracking_mouse = false;
        }
    }

    virtual bool Showing() override {
        return m_showing;
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

    virtual void SetDisplaySize(int size) override {
        m_metrics = GetMetricsForSize(static_cast<DisplaySize>(size));
        DiscardGraphicsResources();
    }

    virtual void SetAppearance(ColorScheme const &colors) override {
        m_colors = colors;
    }

  private:
    void OnCreate() {
        m_d2d1 = Graphics::CreateD2D1Factory();
        m_dwrite = Graphics::CreateDwriteFactory();
        m_dpi = ::GetDpiForWindow(m_hwnd);
        m_dpi_parent = ::GetDpiForWindow(m_hwnd_parent);
        m_scale = static_cast<float>(m_dpi / USER_DEFAULT_SCREEN_DPI);
        OnMonitorSizeChange();
    }

    void OnDpiChanged(WORD dpi, RECT *pNewSize) {
        if (m_target) {
            m_target->SetDpi(dpi, dpi);
        }
        m_dpi = dpi;
        m_dpi_parent = ::GetDpiForWindow(m_hwnd_parent);
        m_scale = static_cast<float>(m_dpi_parent) / USER_DEFAULT_SCREEN_DPI;
        auto width = pNewSize->right - pNewSize->left;
        auto height = pNewSize->bottom - pNewSize->top;
        KHIIN_INFO("width {}, height {}", width, height);
        ::SetWindowPos(m_hwnd, NULL, pNewSize->left, pNewSize->top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void OnResize(unsigned int width, unsigned int height) {
        EnsureRenderTarget();
        m_target->Resize(D2D1_SIZE_U{width, height});
    }

    bool GetPointInClientDp(UINT x, UINT y, Point &pt) {
        auto rect = RECT{};
        ::GetClientRect(m_hwnd, &rect);
        POINT pt_px{static_cast<LONG>(x), static_cast<LONG>(y)};

        pt.x = static_cast<int>(x / m_scale);
        pt.y = static_cast<int>(y / m_scale);

        if (!::PtInRect(&rect, pt_px)) {
            return false;
        } else {
            return true;
        }
    }

    void OnMouseMove(UINT x, UINT y) {
        auto pt = Point();
        auto in_window = GetPointInClientDp(x, y, pt);

        if (!in_window) {
            m_mouse_focused_id = -1;
            return;
        }

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

    void OnMouseLeave() {
        m_tracking_mouse = false;
        m_mouse_focused_id = -1;
    }

    bool HandleClick(UINT x, UINT y) {
        auto pt = Point();
        auto in_window = GetPointInClientDp(x, y, pt);

        if (!in_window) {
            Hide();
            return false;
        }

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

    void EnsureRenderTarget() {
        if (m_d2d1 && m_hwnd && !m_target) {
            m_target = Graphics::CreateRenderTarget(m_d2d1, m_hwnd);
            m_target->SetDpi(static_cast<float>(m_dpi), static_cast<float>(m_dpi));
        }
    }

    void EnsureTextFormat() {
        if (!m_textformat) {
            check_hresult(m_dwrite->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR,
                                                     DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                     m_metrics.font_size, L"en-us", m_textformat.put()));
            check_hresult(m_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            check_hresult(m_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
        }

        if (!m_textformat_sm) {
            check_hresult(m_dwrite->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR,
                                                     DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                     m_metrics.font_size_sm, L"en-us", m_textformat_sm.put()));
            check_hresult(m_textformat_sm->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            check_hresult(m_textformat_sm->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
        }
    }

    void EnsureBrush() {
        if (!m_brush) {
            check_hresult(m_target->CreateSolidColorBrush(m_colors.text, m_brush.put()));
        }
    }

    void CreateGraphicsResources() {
        if (!m_textformat) {
            check_hresult(m_dwrite->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR,
                                                     DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                     m_metrics.font_size, L"en-us", m_textformat.put()));
            check_hresult(m_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            check_hresult(m_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
        }

        if (!m_brush) {
            check_hresult(m_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), m_brush.put()));
        }
    }

    void DiscardGraphicsResources() {
        m_target = nullptr;
        m_brush = nullptr;
        m_textformat = nullptr;
        m_textformat_sm = nullptr;
    }

    void OnMonitorSizeChange() {
        auto hmon = ::MonitorFromWindow(m_hwnd_parent, MONITOR_DEFAULTTONEAREST);
        auto info = MONITORINFO();
        info.cbSize = sizeof(MONITORINFO);
        ::GetMonitorInfo(hmon, &info);
        m_max_width = info.rcMonitor.right;
        m_max_height = info.rcMonitor.bottom;
    }

    uint32_t MinColWidth() {
        return (m_display_mode == DisplayMode::Grid) ? m_metrics.min_col_w_multi : m_metrics.min_col_w_single;
    }

    void AddLayoutToGrid(int row, int col, Candidate const *candidate) {
        auto &value = Utils::Widen(candidate->value());
        auto layout = com_ptr<IDWriteTextLayout>();
        check_hresult(m_dwrite->CreateTextLayout(value.c_str(), static_cast<UINT32>(value.size() + 1),
                                                 m_textformat.get(), static_cast<float>(m_max_width),
                                                 static_cast<float>(m_max_height), layout.put()));

        DWRITE_TEXT_METRICS dwtm;
        check_hresult(layout->GetMetrics(&dwtm));

        m_layout_grid.EnsureColumnWidth(col, static_cast<int>(dwtm.width + m_metrics.qs_col_w));
        m_layout_grid.EnsureRowHeight(static_cast<int>(dwtm.height));
        m_layout_grid.AddItem(row, col, CandidateLayout{candidate, std::move(layout)});
    }

    void CalculateLayout() {
        if (!m_dwrite || !m_candidate_grid) {
            return;
        }

        EnsureTextFormat();

        int n_cols = static_cast<int>(m_candidate_grid->size());
        int n_rows = static_cast<int>(m_candidate_grid->at(0).size());

        m_layout_grid = CandidateLayoutGrid(n_rows, n_cols, MinColWidth());
        m_layout_grid.SetRowPadding(static_cast<int>(m_metrics.padding));

        for (auto col_idx = 0; col_idx < n_cols; ++col_idx) {
            auto &candidates = m_candidate_grid->at(col_idx);
            for (auto row_idx = 0; row_idx < candidates.size(); ++row_idx) {
                auto candidate = candidates[row_idx];
                AddLayoutToGrid(row_idx, col_idx, candidate);
            }
        }

        m_metrics.row_height = m_layout_grid.row_height();
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

    void DrawQuickSelect(std::wstring label, float left, float top) {
        auto layout = com_ptr<IDWriteTextLayout>();
        check_hresult(m_dwrite->CreateTextLayout(label.c_str(), static_cast<UINT32>(label.size() + 1),
                                                 m_textformat_sm.get(), static_cast<float>(m_metrics.qs_col_w),
                                                 m_metrics.row_height, layout.put()));
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
        m_target->Clear(m_colors.bg);
        EnsureBrush();

        auto qs_label = 1;
        for (auto col_idx = 0; col_idx < m_layout_grid.cols(); ++col_idx) {
            auto &col = m_layout_grid.items[col_idx];
            for (auto row_idx = 0; row_idx < col.size(); ++row_idx) {
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
                    DrawQuickSelect(std::to_wstring(qs_label), left, top);
                    ++qs_label;
                }

                DrawCandidate(value, left, top);
            }
        }
    }

    void Render() {
        EnsureRenderTarget();

        PAINTSTRUCT ps;
        ::BeginPaint(m_hwnd, &ps);

        m_target->BeginDraw();
        Draw();
        auto hr = m_target->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
            DiscardGraphicsResources();
        }
        ::EndPaint(m_hwnd, &ps);
    }

    bool m_showing = false;
    HWND m_hwnd_parent = nullptr;
    bool m_tracking_mouse = false;

    com_ptr<ID2D1Factory1> m_d2d1 = nullptr;
    com_ptr<IDWriteFactory3> m_dwrite = nullptr;
    com_ptr<ID2D1HwndRenderTarget> m_target = nullptr;
    com_ptr<ID2D1SolidColorBrush> m_brush = nullptr;
    com_ptr<IDWriteTextFormat> m_textformat = nullptr;
    com_ptr<IDWriteTextFormat> m_textformat_sm = nullptr;

#pragma warning(push)
#pragma warning(disable : 26812)
    DWM_WINDOW_CORNER_PREFERENCE m_border_radius = DWMWCP_ROUND;
#pragma warning(pop)
    RECT m_border_thickness{};

    uint32_t m_max_width = 0;
    uint32_t m_max_height = 0;
    uint32_t m_dpi_parent = USER_DEFAULT_SCREEN_DPI;
    uint32_t m_dpi = USER_DEFAULT_SCREEN_DPI;
    float m_scale = 1.0f;

    // Candidate rendering
    CandidateLayoutGrid m_layout_grid;
    CandidateGrid *m_candidate_grid = nullptr;
    RECT m_text_rect;
    CWndMetrics m_metrics = GetMetricsForSize(DisplaySize::S);
    ColorScheme m_colors;
    DisplayMode m_display_mode = DisplayMode::ShortColumn;
    int m_focused_id = -1;
    size_t m_quickselect_col = 0;
    bool m_quickselect_active = false;
    int m_mouse_focused_id = -1;

    std::vector<CandidateSelectListener *> m_focus_listeners;
};

} // namespace

std::wstring kCandidateWindowClassName = L"CandidateWindow";

GUID kCandidateWindowGuid // 829893fa-728d-11ec-8c6e-e0d46491b35a
    = {0x829893fa, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

CandidateWindow *CandidateWindow::Create(HWND parent) {
    auto impl = new CandidateWindowImpl();
    impl->Create(parent);
    return impl;
}

} // namespace khiin::win32
