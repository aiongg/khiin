#include "pch.h"

#include <algorithm>

#include "CandidateWindow.h"

#include "Utils.h"

namespace khiin::win32 {

namespace {

using namespace messages;

constexpr size_t kExpandedCols = 4;
constexpr size_t kShortColSize = 5;
constexpr size_t kLongColSize = 9;

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

float CenterY(IDWriteTextLayout *layout, float available_height) {
    DWRITE_TEXT_METRICS metrics;
    winrt::check_hresult(layout->GetMetrics(&metrics));
    return (available_height - metrics.height) / 2;
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
        auto left = text_rect_px.left - qs_col_width;
        auto top = text_rect_px.bottom + static_cast<int>(padding_sm);
        ::SetWindowPos(m_hwnd, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
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
                                                               font_size, L"en-us", m_textformat.put()));
            winrt::check_hresult(m_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
            winrt::check_hresult(m_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
        }
    }

    void EnsureBrush() {
        D(__FUNCTIONW__);
        if (!m_brush) {
            winrt::check_hresult(m_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), m_brush.put()));
        }
    }

    void CreateGraphicsResources() {
        D(__FUNCTIONW__);
        if (!m_textformat) {
            winrt::check_hresult(m_dwfactory->CreateTextFormat(
                L"Kozuka Gothic Pr6N R", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, font_size, L"en-us", m_textformat.put()));
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

        auto longest_col = m_candidate_grid->at(0).size();
        auto min_col_width = (m_display_mode == DisplayMode::Expanded) ? min_col_width_expanded : min_col_width_single;
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
                DWRITE_TEXT_METRICS metrics;
                winrt::check_hresult(layout->GetMetrics(&metrics));
                column_width = std::max(column_width, metrics.width);
                layout_col.push_back(CandidateLayout{candidate, std::move(layout)});
                row_height = std::max(row_height, metrics.height);
            }

            column_width = std::max(column_width, static_cast<float>(min_col_width)) + qs_col_width;

            if (column_width > (min_col_width + qs_col_width - padding)) {
                column_width += padding;
            }

            D("Col width: ", column_width);
            m_col_widths.push_back(column_width);
            page_width += column_width;
            m_candidate_layouts.push_back(std::move(layout_col));
        }

        row_height += padding;
        m_row_height = row_height;
        auto page_height = longest_col * row_height;
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

    void SetBrushColor(D2D1::ColorF color) {
        D(__FUNCTIONW__);
        m_brush->SetColor(D2D1::ColorF(color));
    }

    void Draw() {
        D(__FUNCTIONW__);
        m_target->Clear(bg_color);

        EnsureBrush();
        SetBrushColor(text_color);
        auto row_top = 0.0f;
        auto row_left = 0.0f;

        auto origin = D2D1::Point2F(qs_col_width, padding / 2);
        D("origin: ", qs_col_width, ",", padding);

        auto col_idx = 0;
        for (auto &column : m_candidate_layouts) {
            auto qs_number = 1;
            for (auto &row : column) {
                if (row.candidate->id() == m_focused_id) {
                    auto highlight_rect =
                        D2D1::RectF(row_left, row_top, m_col_widths.at(col_idx) - padding, row_top + m_row_height);
                    auto highlight_roundrect = D2D1::RoundedRect(highlight_rect, padding_sm, padding_sm);
                    SetBrushColor(bg_selected_color);
                    m_target->FillRoundedRectangle(highlight_roundrect, m_brush.get());

                    auto highlight_dot = D2D1::RoundedRect(
                        D2D1::RectF(row_left + m_marker_width, row_top + (m_row_height - m_marker_height) / 2,
                                    row_left + m_marker_width * 2,
                                    row_top + (m_row_height - m_marker_height) / 2 + m_marker_height),
                        2.0f, 2.0f);
                    SetBrushColor(highlight_color);
                    m_target->FillRoundedRectangle(highlight_dot, m_brush.get());
                    SetBrushColor(text_color);
                }

                if (col_idx == m_quickselect_col) {
                    auto qs_str = std::to_wstring(qs_number);
                    auto qs_layout = winrt::com_ptr<IDWriteTextLayout>();
                    winrt::check_hresult(
                        m_dwfactory->CreateTextLayout(qs_str.c_str(), static_cast<UINT32>(qs_str.size() + 1),
                                                      m_textformat.get(), qs_col_width, m_row_height, qs_layout.put()));
                    origin.x = row_left + m_marker_width * 2 + padding;
                    origin.y = row_top + CenterY(qs_layout.get(), m_row_height);
                    if (!m_quickselect_active) {
                        SetBrushColor(qs_disabled_color);
                    }
                    m_target->DrawTextLayout(origin, qs_layout.get(), m_brush.get());
                    if (!m_quickselect_active) {
                        SetBrushColor(text_color);
                    }
                    ++qs_number;
                }

                // uncomment to draw boxes around each candidate
                // DWRITE_TEXT_METRICS metrics;
                // row_layout->GetMetrics(&metrics);
                // D("box: ", metrics.left, ", ", metrics.top, ", ", metrics.width, ", ", metrics.height,
                //  ", layoutWidth: ", metrics.layoutWidth);
                // auto box = D2D1::RectF((int)origin.x, (int)origin.y, (int)origin.x + (int)metrics.width,
                //                       (int)origin.y + (int)metrics.height);
                // m_target->DrawRectangle(&box, m_brush.get(), 1, NULL);

                origin.x = row_left + qs_col_width;
                origin.y = row_top + CenterY(row.layout.get(), m_row_height);
                m_target->DrawTextLayout(origin, row.layout.get(), m_brush.get(),
                                         D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
                row_top += m_row_height;
            }
            row_left += m_col_widths.at(col_idx);
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

    D2D1::ColorF text_color = D2D1::ColorF(D2D1::ColorF::Black);
    D2D1::ColorF bg_selected_color = D2D1::ColorF(0.90f, 0.90f, 0.90f);
    D2D1::ColorF highlight_color = D2D1::ColorF(D2D1::ColorF::CornflowerBlue);
    D2D1::ColorF bg_color = D2D1::ColorF(0.97f, 0.97f, 0.97f);
    D2D1::ColorF qs_disabled_color = D2D1::ColorF(D2D1::ColorF::Gray);

#pragma warning(push)
#pragma warning(disable : 26812)
    DWM_WINDOW_CORNER_PREFERENCE m_border_radius = DWMWCP_ROUND;
#pragma warning(pop)
    RECT m_border_thickness{};

    unsigned int m_max_width = 0;
    unsigned int m_max_height = 0;
    unsigned int m_dpi_parent = USER_DEFAULT_SCREEN_DPI;
    unsigned int m_dpi = USER_DEFAULT_SCREEN_DPI;
    float m_scale = 1.0f;
    float padding = 8.0f;
    float padding_sm = 4.0f;
    float m_marker_width = 4.0f;
    float m_marker_height = 16.0f;
    float font_size = 16.0f;
    float m_row_height = font_size + padding;
    unsigned int min_col_width_single = 160;
    unsigned int min_col_width_expanded = 80;
    unsigned int qs_col_width = padding_sm + m_marker_width + padding * 2 + 12;
    unsigned int n_cols_expanded = 4;
    unsigned int qs_col = 0;
    unsigned int page_idx = 0;
    int selected_idx = -1;
    unsigned int short_col_size = 5;
    unsigned int long_col_size = 9;
    unsigned int expanded_n_cols = 4;
    std::vector<unsigned int> m_col_widths{};

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
