#include "pch.h"

#include <algorithm>

#include "CandidateWindow.h"

#include "Utils.h"

namespace khiin::win32 {

namespace {

using namespace messages;
using uint = unsigned int;

constexpr float kPadding = 8.0f;
constexpr float kPaddingSmall = 4.0f;
constexpr float kMarkerWidth = 4.0f;
constexpr float kMarkerHeight = 16.0f;
constexpr float kFontSize = 16.0f;
constexpr float kFontSizeSmall = 16.0f;
constexpr float kRowHeight = kFontSize + kPadding;
constexpr uint min_col_width_single = 160;
constexpr uint min_col_width_expanded = 80;
constexpr uint kQsColWidth = kPaddingSmall * 2 + kMarkerWidth + kPadding * 2 + kFontSize;

struct CWndMetrics {
    float padding = kPadding;
    float padding_sm = kPaddingSmall;
    float marker_w = kMarkerWidth;
    float marker_h = kMarkerHeight;
    float font_size = kFontSize;
    float font_size_sm = kFontSizeSmall;
    float row_height = kRowHeight;
    uint min_col_w_single = 160;
    uint min_col_w_multi = 80;
    uint qs_col_w = kQsColWidth;
};

struct CWndColors {
    D2D1::ColorF text = D2D1::ColorF(D2D1::ColorF::Black);
    D2D1::ColorF text_disabled = D2D1::ColorF(D2D1::ColorF::Gray);
    D2D1::ColorF text_extended = D2D1::ColorF(D2D1::ColorF::DarkGray);
    D2D1::ColorF bg = D2D1::ColorF(0.97f, 0.97f, 0.97f);
    D2D1::ColorF bg_selected = D2D1::ColorF(0.90f, 0.90f, 0.90f);
    D2D1::ColorF accent = D2D1::ColorF(D2D1::ColorF::CornflowerBlue);
    D2D1::ColorF hint = D2D1::ColorF(D2D1::ColorF::ForestGreen);
};

CWndColors const kLightColorScheme = CWndColors();
CWndColors const kDarkColorScheme =
    CWndColors{D2D1::ColorF(0.98f, 0.98f, 0.98f),      D2D1::ColorF(D2D1::ColorF::LightGray),
               D2D1::ColorF(D2D1::ColorF::WhiteSmoke),   D2D1::ColorF(0.17f, 0.17f, 0.17f),
               D2D1::ColorF(0.12f, 0.12f, 0.12f),      D2D1::ColorF(D2D1::ColorF::LightSkyBlue),
               D2D1::ColorF(D2D1::ColorF::LightGoldenrodYellow)};

CWndMetrics GetMetricsForSize(DisplaySize size) {
    auto metrics = CWndMetrics();
    switch (size) {
    case DisplaySize::M:
        metrics.padding = 10.0f;
        metrics.padding_sm = 5.0f;
        metrics.font_size = 20.0f;
        break;
    case DisplaySize::L:
        metrics.padding = 12.0f;
        metrics.padding_sm = 6.0f;
        metrics.font_size = 24.0f;
        break;
    case DisplaySize::XL:
        metrics.padding = 14.0f;
        metrics.padding_sm = 7.0f;
        metrics.font_size = 28.0f;
        break;
    case DisplaySize::XXL:
        metrics.padding = 16.0f;
        metrics.padding_sm = 8.0f;
        metrics.font_size = 32.0f;
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

winrt::com_ptr<ID2D1Factory1> CreateD2D1Factory() {
    auto factory = winrt::com_ptr<ID2D1Factory1>();
    winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.put()));
    return factory;
}

winrt::com_ptr<IDWriteFactory3> CreateDwriteFactory() {
    auto factory = winrt::com_ptr<IDWriteFactory3>();
    winrt::check_hresult(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3),
                                               reinterpret_cast<IUnknown **>(factory.put())));
    return factory;
}

winrt::com_ptr<ID2D1HwndRenderTarget> CreateRenderTarget(winrt::com_ptr<ID2D1Factory1> const &factory,
                                                         HWND const &hwnd) {
    WINRT_ASSERT(factory);
    RECT rc;
    ::GetClientRect(hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
    auto target = winrt::com_ptr<ID2D1HwndRenderTarget>();
    winrt::check_hresult(factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                         D2D1::HwndRenderTargetProperties(hwnd, size), target.put()));
    return target;
}

float CenterTextLayoutY(IDWriteTextLayout *layout, float available_height) {
    DWRITE_TEXT_METRICS dwt_metrics;
    winrt::check_hresult(layout->GetMetrics(&dwt_metrics));
    return (available_height - dwt_metrics.height) / 2;
}

struct CandidateLayout {
    Candidate const *candidate = nullptr;
    winrt::com_ptr<IDWriteTextLayout> layout = winrt::com_ptr<IDWriteTextLayout>();
};

using CandidateLayoutGrid = std::vector<std::vector<CandidateLayout>>;

class CandidateWindowImpl : public CandidateWindow {
  public:
    void Create(HWND parent) {
        m_hwnd_parent = parent;

        D(__FUNCTIONW__);
        Create_(NULL, // clang-format off
            kDwStyle,
            kDwExStyle,
            0, 0, 100, 100,
            m_hwnd_parent,
            NULL); // clang-format on
    }

    virtual LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override {
        switch (uMsg) {
        case WM_NCCREATE: {
            D("WM_NCCREATE");
#pragma warning(push)
#pragma warning(disable : 26812)
            ::DwmSetWindowAttribute(m_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &m_border_radius,
                                    sizeof(DWM_WINDOW_CORNER_PREFERENCE));
#pragma warning(pop)
            break;
        }
        case WM_CREATE: {
            D("WM_CREATE");
            try {
                OnCreate();
                return 0;
            } catch (...) {
                return -1;
            }
        }
        case WM_DISPLAYCHANGE: {
            D("WM_DISPLAYCHANGE");
            OnDisplayChange();
            break;
        }
        case WM_DPICHANGED: {
            D("WM_DPICHANGED");
            OnDpiChanged(HIWORD(wParam), (RECT *)lParam);
            return 0;
        }
        case WM_NCCALCSIZE: {
            D("WM_NCCALCSIZE");
            break;
        }
        case WM_NCPAINT: {
            D("WM_NCPAINT");
            break;
        }
        case WM_NCACTIVATE: {
            D("WM_NCACTIVATE");
            break;
        }
        case WM_ACTIVATE: {
            D("WM_ACTIVATE");
            break;
        }
        case WM_PAINT: {
            D("WM_PAINT");
            Render();
            return 0;
        }
        case WM_DESTROY: {
            D("WM_DESTROY");
            break;
        }
        case WM_NCDESTROY: {
            D("WM_NCDESTROY");
            break;
        }
        case WM_MOVE: {
            D("WM_MOVE");
            break;
        }
        case WM_SIZE: {
            D("WM_SIZE");
            OnResize(LOWORD(lParam), HIWORD(lParam));
            break;
        }
        case WM_WINDOWPOSCHANGING: {
            D("WM_WINDOWPOSCHANGING");
            break;
        }
        case WM_WINDOWPOSCHANGED: {
            D("WM_WINDOWPOSCHANGED");
            break;
        }
        case WM_SHOWWINDOW: {
            D("WM_SHOWWINDOW");
            break;
        }
        case WM_ERASEBKGND: {
            D("WM_ERASEBKGND");
            break;
        }
        default: {
            D(__FUNCTIONW__, " (", uMsg, ")");
            break;
        }
        }

        return ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }

    virtual std::wstring &class_name() const override {
        return kCandidateWindowClassName;
    }

    virtual void Show() override {
        if (m_showing) {
            return;
        }

        ::ShowWindow(m_hwnd, SW_SHOWNA);
        m_showing = true;
    }

    virtual void Hide() override {
        if (!m_showing) {
            return;
        }

        ::ShowWindow(m_hwnd, SW_HIDE);
        m_showing = false;
    }

    virtual bool Showing() override {
        return m_showing;
    }

    virtual void SetCandidates(DisplayMode display_mode, CandidateGrid *candidate_grid, int focused_id, size_t qs_col,
                               bool qs_active) override {
        D(__FUNCTIONW__);
        m_candidate_grid = candidate_grid;
        m_display_mode = display_mode;
        m_focused_id = focused_id;
        m_quickselect_col = qs_col;
        m_quickselect_active = qs_active;
        CalculateLayout();
    }

    virtual void SetScreenCoordinates(RECT text_rect_px) override {
        D(__FUNCTIONW__);
        if (Showing()) {
            return;
        }
        auto left = text_rect_px.left - metrics.qs_col_w;
        auto top = text_rect_px.bottom + static_cast<int>(metrics.padding);
        ::SetWindowPos(m_hwnd, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    virtual void SetDisplaySize(DisplaySize display_size) override {
        metrics = GetMetricsForSize(display_size);
        DiscardGraphicsResources();
    }

  private:
    void OnCreate() {
        D(__FUNCTIONW__);
        m_d2factory = CreateD2D1Factory();
        m_dwfactory = CreateDwriteFactory();
        m_dpi = ::GetDpiForWindow(m_hwnd);
        m_dpi_parent = ::GetDpiForWindow(m_hwnd_parent);
        m_scale = static_cast<float>(m_dpi / USER_DEFAULT_SCREEN_DPI);
        OnDisplayChange();
    }

    void EnsureRenderTarget() {
        D(__FUNCTIONW__);
        if (m_d2factory && m_hwnd && !m_target) {
            m_target = CreateRenderTarget(m_d2factory, m_hwnd);
            m_target->SetDpi(static_cast<float>(m_dpi), static_cast<float>(m_dpi));
        }
    }

    void EnsureTextFormat() {
        D(__FUNCTIONW__);
        if (!m_textformat) {
            winrt::check_hresult(m_dwfactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR,
                                                               DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                               metrics.font_size, L"en-us", m_textformat.put()));
            winrt::check_hresult(m_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            winrt::check_hresult(m_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
        }

        if (!m_textformat_sm) {
            winrt::check_hresult(m_dwfactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR,
                                                               DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                               metrics.font_size_sm, L"en-us", m_textformat_sm.put()));
            winrt::check_hresult(m_textformat_sm->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            winrt::check_hresult(m_textformat_sm->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
        }
    }

    void EnsureBrush() {
        D(__FUNCTIONW__);
        if (!m_brush) {
            winrt::check_hresult(m_target->CreateSolidColorBrush(colors.text, m_brush.put()));
        }
    }

    void CreateGraphicsResources() {
        D(__FUNCTIONW__);
        if (!m_textformat) {
            winrt::check_hresult(m_dwfactory->CreateTextFormat(
                L"Kozuka Gothic Pr6N R", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, metrics.font_size, L"en-us", m_textformat.put()));
            winrt::check_hresult(m_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            winrt::check_hresult(m_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
        }

        if (!m_brush) {
            winrt::check_hresult(m_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), m_brush.put()));
        }
    }

    void DiscardGraphicsResources() {
        D(__FUNCTIONW__);
        m_target = nullptr;
        m_brush = nullptr;
        m_textformat = nullptr;
        m_textformat_sm = nullptr;
        m_candidate_layouts.clear();
    }

    void OnDisplayChange() {
        m_max_width = ::GetSystemMetrics(SM_CXFULLSCREEN);
        m_max_height = ::GetSystemMetrics(SM_CYFULLSCREEN);
    }

    void CalculateLayout() {
        D(__FUNCTIONW__);
        if (!m_dwfactory || !m_candidate_grid) {
            return;
        }

        EnsureTextFormat();

        m_candidate_layouts.clear();
        m_col_widths.clear();

        auto min_col_width =
            (m_display_mode == DisplayMode::Expanded) ? metrics.min_col_w_multi : metrics.min_col_w_single;
        auto page_width = 0.0f;
        auto row_height = 0.0f;

        for (auto &grid_col : *m_candidate_grid) {
            auto layout_col = std::vector<CandidateLayout>();
            auto column_width = 0.0f;

            for (auto &candidate : grid_col) {
                auto &value = Utils::Widen(candidate->value());
                auto layout = winrt::com_ptr<IDWriteTextLayout>();
                D(value, " (", value.size(), ")");
                winrt::check_hresult(m_dwfactory->CreateTextLayout(value.c_str(), static_cast<UINT32>(value.size() + 1),
                                                                   m_textformat.get(), static_cast<float>(m_max_width),
                                                                   row_height, layout.put()));
                DWRITE_TEXT_METRICS dwtm;
                winrt::check_hresult(layout->GetMetrics(&dwtm));
                column_width = std::max(column_width, dwtm.width);
                layout_col.push_back(CandidateLayout{candidate, std::move(layout)});
                row_height = std::max(row_height, dwtm.height);
            }

            column_width = std::max(column_width, static_cast<float>(min_col_width)) + metrics.qs_col_w;

            if (column_width > (min_col_width + metrics.qs_col_w - metrics.padding)) {
                column_width += metrics.padding;
            }

            D("Col width: ", column_width);
            m_col_widths.push_back(column_width);
            page_width += column_width;
            m_candidate_layouts.push_back(std::move(layout_col));
        }

        row_height += metrics.padding;
        metrics.row_height = row_height;
        auto page_height = m_candidate_grid->at(0).size() * row_height;
        D("page_height: ", page_height, " (", page_height * m_scale, ")");
        D("page_width: ", page_width, " (", page_width * m_scale, ")");
        ::SetWindowPos(m_hwnd, NULL, 0, 0, static_cast<int>(page_width * m_scale),
                       static_cast<int>(page_height * m_scale), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
        ::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }

    void OnDpiChanged(WORD dpi, RECT *pNewSize) {
        D(__FUNCTIONW__);
        if (m_target) {
            m_target->SetDpi(dpi, dpi);
        }
        m_dpi = dpi;
        m_dpi_parent = ::GetDpiForWindow(m_hwnd_parent);
        m_scale = static_cast<float>(m_dpi_parent) / USER_DEFAULT_SCREEN_DPI;
        auto width = pNewSize->right - pNewSize->left;
        auto height = pNewSize->bottom - pNewSize->top;
        D("W", width, "H", height);
        ::SetWindowPos(m_hwnd, NULL, pNewSize->left, pNewSize->top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void OnResize(unsigned int width, unsigned int height) {
        D(__FUNCTIONW__);
        EnsureRenderTarget();
        m_target->Resize(D2D1_SIZE_U{width, height});
    }

    void DrawFocused(float left, float top, uint col_width) {
        m_brush->SetColor(colors.bg_selected);
        m_target->FillRoundedRectangle(
            D2D1::RoundedRect(D2D1::RectF(left, top, col_width - metrics.padding, top + metrics.row_height),
                              metrics.padding_sm, metrics.padding_sm),
            m_brush.get());

        m_brush->SetColor(colors.accent);
        m_target->FillRoundedRectangle(
            D2D1::RoundedRect(D2D1::RectF(left + metrics.marker_w, top + (metrics.row_height - metrics.marker_h) / 2,
                                          left + metrics.marker_w * 2,
                                          top + (metrics.row_height - metrics.marker_h) / 2 + metrics.marker_h),
                              2.0f, 2.0f),
            m_brush.get());
    }

    void DrawQuickSelect(int number, float left, float top) {
        auto val = std::to_wstring(number);
        auto layout = winrt::com_ptr<IDWriteTextLayout>();
        winrt::check_hresult(m_dwfactory->CreateTextLayout(val.c_str(), static_cast<UINT32>(val.size() + 1),
                                                           m_textformat_sm.get(), metrics.qs_col_w, metrics.row_height,
                                                           layout.put()));
        auto x = left + metrics.marker_w * 2 + metrics.padding;
        auto y = top + CenterTextLayoutY(layout.get(), metrics.row_height);
        if (m_quickselect_active) {
            m_brush->SetColor(colors.text);
        } else {
            m_brush->SetColor(colors.text_disabled);
        }
        m_target->DrawTextLayout(D2D1::Point2F(x, y), layout.get(), m_brush.get());
    }

    void DrawCandidate(CandidateLayout &candidate, float left, float top) {
        auto x = left + metrics.qs_col_w;
        auto y = top + CenterTextLayoutY(candidate.layout.get(), metrics.row_height);
        m_brush->SetColor(colors.text);
        m_target->DrawTextLayout(D2D1::Point2F(x, y), candidate.layout.get(), m_brush.get(),
                                 D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
    }

    void Draw() {
        D(__FUNCTIONW__);
        m_target->Clear(colors.bg);

        EnsureBrush();
        auto row_top = 0.0f;
        auto row_left = 0.0f;

        auto origin = D2D1::Point2F(metrics.qs_col_w, metrics.padding / 2);
        D("origin: ", metrics.qs_col_w, ",", metrics.padding);

        auto col_idx = 0;
        for (auto &column : m_candidate_layouts) {
            auto qs_number = 1;
            for (auto &row : column) {
                if (row.candidate->id() == m_focused_id) {
                    DrawFocused(row_left, row_top, m_col_widths.at(col_idx));
                }

                if (col_idx == m_quickselect_col) {
                    DrawQuickSelect(qs_number, row_left, row_top);
                    ++qs_number;
                }

                DrawCandidate(row, row_left, row_top);
                row_top += metrics.row_height;
            }
            row_left += m_col_widths.at(col_idx);
            ++col_idx;
        }
    }

    void Render() {
        D(__FUNCTIONW__);
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

    winrt::com_ptr<ID2D1Factory1> m_d2factory = nullptr;
    winrt::com_ptr<IDWriteFactory3> m_dwfactory = nullptr;
    winrt::com_ptr<ID2D1HwndRenderTarget> m_target = nullptr;
    winrt::com_ptr<ID2D1SolidColorBrush> m_brush = nullptr;
    winrt::com_ptr<IDWriteTextFormat> m_textformat = nullptr;
    winrt::com_ptr<IDWriteTextFormat> m_textformat_sm = nullptr;

#pragma warning(push)
#pragma warning(disable : 26812)
    DWM_WINDOW_CORNER_PREFERENCE m_border_radius = DWMWCP_ROUND;
#pragma warning(pop)
    RECT m_border_thickness{};

    uint m_max_width = 0;
    uint m_max_height = 0;
    uint m_dpi_parent = USER_DEFAULT_SCREEN_DPI;
    uint m_dpi = USER_DEFAULT_SCREEN_DPI;
    float m_scale = 1.0f;

    // Candidate rendering
    CWndMetrics metrics = GetMetricsForSize(DisplaySize::S);
    CWndColors colors = kDarkColorScheme;
    std::vector<uint> m_col_widths{};
    CandidateGrid *m_candidate_grid = nullptr;
    DisplayMode m_display_mode = DisplayMode::Short;
    int m_focused_id = -1;
    CandidateLayoutGrid m_candidate_layouts = {};
    size_t m_quickselect_col = 0;
    bool m_quickselect_active = false;
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
