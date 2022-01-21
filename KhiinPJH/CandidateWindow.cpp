#include "pch.h"

#include "CandidateWindow.h"

namespace Khiin {

static inline constexpr int kCornerRadius = 4;

static inline int divide_ceil(int x, int y) {
    return x / y + (x % y != 0);
}

std::wstring kCandidateWindowClassName = L"CandidateWindow";
GUID kCandidateWindowGuid // 829893fa-728d-11ec-8c6e-e0d46491b35a
    = {0x829893fa, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

namespace {

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

} // namespace

CandidateWindow::CandidateWindow(HWND parent) : m_hwnd_parent(parent) {}

LRESULT __stdcall CandidateWindow::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_NCCREATE: {
        D("WM_NCCREATE");
        ::DwmSetWindowAttribute(m_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &m_border_radius,
                                sizeof(DWM_WINDOW_CORNER_PREFERENCE));
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

std::wstring &CandidateWindow::class_name() const {
    return kCandidateWindowClassName;
}

static const DWORD kDwStyle = WS_BORDER | WS_POPUP;
static const DWORD kDwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;

void CandidateWindow::Create() {
    D(__FUNCTIONW__);
    Create_(NULL, // clang-format off
            kDwStyle,
            kDwExStyle,
            0, 0, 100, 100,
            m_hwnd_parent,
            NULL); // clang-format on
}

// void CandidateWindow::Destroy() {
//    Destroy_();
//}

void CandidateWindow::Show() {
    if (m_showing) {
        return;
    }

    ::ShowWindow(m_hwnd, SW_SHOWNA);
    m_showing = true;
}

void CandidateWindow::Hide() {
    if (!m_showing) {
        return;
    }

    ::ShowWindow(m_hwnd, SW_HIDE);
    m_showing = false;
}

bool CandidateWindow::showing() {
    return m_showing;
}

void CandidateWindow::SetCandidates(std::vector<std::wstring> *candidates) {
    D(__FUNCTIONW__);
    this->candidates = candidates;
    CalculateLayout();
}

void CandidateWindow::SetScreenCoordinates(RECT text_rect_px) {
    D(__FUNCTIONW__);
    if (showing()) {
        return;
    }
    auto left = text_rect_px.left - qs_col_width;
    auto top = text_rect_px.bottom + static_cast<int>(padding_sm);
    ::SetWindowPos(m_hwnd, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CandidateWindow::OnCreate() {
    D(__FUNCTIONW__);
    m_d2factory = CreateD2D1Factory();
    m_dwfactory = CreateDwriteFactory();
    m_dpi = ::GetDpiForWindow(m_hwnd);
    m_dpi_parent = ::GetDpiForWindow(m_hwnd_parent);
    m_scale = static_cast<float>(m_dpi / USER_DEFAULT_SCREEN_DPI);
    OnDisplayChange();
}

void CandidateWindow::EnsureRenderTarget() {
    D(__FUNCTIONW__);
    if (m_d2factory && m_hwnd && !m_target) {
        m_target = CreateRenderTarget(m_d2factory, m_hwnd);
        m_target->SetDpi(static_cast<float>(m_dpi), static_cast<float>(m_dpi));
    }
}

void CandidateWindow::EnsureTextFormat() {
    D(__FUNCTIONW__);
    if (!m_textformat) {
        winrt::check_hresult(m_dwfactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR,
                                                           DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                           font_size, L"en-us", m_textformat.put()));
        winrt::check_hresult(m_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
        winrt::check_hresult(m_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
    }
}

void CandidateWindow::EnsureBrush() {
    D(__FUNCTIONW__);
    if (!m_brush) {
        winrt::check_hresult(m_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), m_brush.put()));
    }
}

void CandidateWindow::CreateGraphicsResources() {
    D(__FUNCTIONW__);
    if (!m_textformat) {
        winrt::check_hresult(m_dwfactory->CreateTextFormat(L"Kozuka Gothic Pr6N R", NULL, DWRITE_FONT_WEIGHT_REGULAR,
                                                           DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                           font_size, L"en-us", m_textformat.put()));
        winrt::check_hresult(m_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
        winrt::check_hresult(m_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
    }

    if (!m_brush) {
        winrt::check_hresult(m_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), m_brush.put()));
    }
}

void CandidateWindow::DiscardGraphicsResources() {
    D(__FUNCTIONW__);
    m_target = nullptr;
    m_brush = nullptr;
    m_textformat = nullptr;
    candidate_layout_matrix.clear();
}

void CandidateWindow::OnDisplayChange() {
    m_max_width = ::GetSystemMetrics(SM_CXFULLSCREEN);
    m_max_height = ::GetSystemMetrics(SM_CYFULLSCREEN);
}

void CandidateWindow::CalculateLayout() {
    D(__FUNCTIONW__);
    if (!m_dwfactory || !candidates) {
        return;
    }

    EnsureTextFormat();

    candidate_layout_matrix.clear();
    auto n_cols = (display_mode == DisplayMode::Expanded) ? n_cols_expanded : 1;
    auto min_col_width = (display_mode == DisplayMode::Expanded) ? min_col_width_expanded : min_col_width_single;
    auto max_col_size = display_mode == DisplayMode::Short ? short_col_size : long_col_size;
    auto max_page_size = (display_mode == DisplayMode::Expanded ? expanded_n_cols : 1) * max_col_size;
    auto total_candidates_shown = 0;
    auto total_candidates_avail = candidates->size();
    auto total_cols_avail = divide_ceil(total_candidates_avail, max_col_size);
    auto total_pages = divide_ceil(total_candidates_avail, max_page_size);
    page_idx = min(total_pages - 1, page_idx);
    auto candidate_start_idx = page_idx * max_page_size;
    if (page_idx == total_pages - 1) {
        n_cols = min(max_page_size, total_cols_avail);
    }

    m_col_widths.clear();
    auto page_width = 0.0f;
    for (auto col_idx = 0; col_idx < n_cols; ++col_idx) {
        candidate_layout_matrix.push_back(std::vector<winrt::com_ptr<IDWriteTextLayout>>());
        auto &col_layouts = candidate_layout_matrix.back();

        float col_max_width = 0.0f;
        auto col_break = candidate_start_idx + min(total_candidates_avail - candidate_start_idx, max_col_size);
        for (auto row_idx = candidate_start_idx; row_idx < col_break; ++row_idx) {
            auto &cand = candidates->at(row_idx);
            auto layout = winrt::com_ptr<IDWriteTextLayout>();
            winrt::check_hresult(m_dwfactory->CreateTextLayout(cand.data(), static_cast<UINT>(cand.size()),
                                                               m_textformat.get(), static_cast<float>(m_max_width),
                                                               row_height, layout.put()));
            DWRITE_TEXT_METRICS metrics;
            winrt::check_hresult(layout->GetMetrics(&metrics));
            col_max_width = max(col_max_width, metrics.width);
            col_layouts.push_back(std::move(layout));
        }
        candidate_start_idx += max_col_size + 1;
        col_max_width = max(col_max_width, min_col_width) + qs_col_width;
        D("Col width: ", col_max_width);
        m_col_widths.push_back(col_max_width);
        page_width += col_max_width;
    }

    auto page_height = max_col_size * row_height + padding;
    auto scale = m_dpi_parent / USER_DEFAULT_SCREEN_DPI;
    ::SetWindowPos(m_hwnd, NULL, 0, 0, static_cast<int>(page_width * scale), static_cast<int>(page_height * scale),
                   SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CandidateWindow::OnDpiChanged(WORD dpi, RECT *pNewSize) {
    D(__FUNCTIONW__);
    if (m_target) {
        m_target->SetDpi(dpi, dpi);
    }
    m_dpi = dpi;
    m_dpi_parent = ::GetDpiForWindow(m_hwnd_parent);
    m_scale = static_cast<float>(dpi) / USER_DEFAULT_SCREEN_DPI;
    auto width = pNewSize->right - pNewSize->left;
    auto height = pNewSize->bottom - pNewSize->top;
    D("W", width, "H", height);
    ::SetWindowPos(m_hwnd, NULL, pNewSize->left, pNewSize->top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}

void CandidateWindow::OnResize(unsigned int width, unsigned int height) {
    D(__FUNCTIONW__);
    EnsureRenderTarget();
    m_target->Resize(D2D1_SIZE_U{width, height});
}

void CandidateWindow::SetBrushColor(D2D1::ColorF color) {
    D(__FUNCTIONW__);
    m_brush->SetColor(D2D1::ColorF(color));
}

void CandidateWindow::Draw() {
    D(__FUNCTIONW__);
    m_target->Clear(bg_color);

    EnsureBrush();
    SetBrushColor(text_color);
    auto origin = D2D1::Point2F(qs_col_width, padding);

    auto col_idx = 0;
    for (auto &column : candidate_layout_matrix) {
        for (auto &row_layout : column) {
            // uncomment to draw boxes around each candidate
            // DWRITE_TEXT_METRICS metrics;
            // row_layout->GetMetrics(&metrics);
            // D("box: ", metrics.left, ", ", metrics.top, ", ", metrics.width, ", ", metrics.height,
            //  ", layoutWidth: ", metrics.layoutWidth);
            // auto box = D2D1::RectF((int)origin.x, (int)origin.y, (int)origin.x + (int)metrics.width,
            //                       (int)origin.y + (int)metrics.height);
            // m_target->DrawRectangle(&box, m_brush.get(), 1, NULL);

            m_target->DrawTextLayout(origin, row_layout.get(), m_brush.get(), D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
            origin.y += row_height;
        }
        origin.x += m_col_widths.at(col_idx);
        origin.y = padding;
    }
}

void CandidateWindow::Render() {
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

} // namespace Khiin
