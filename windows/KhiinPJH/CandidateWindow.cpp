#include "pch.h"

#include <algorithm>

#include "CandidateWindow.h"

#include "Utils.h"

namespace {

static inline constexpr int kCornerRadius = 4;

static inline auto divide_ceil(unsigned int x, unsigned int y) {
    return x / y + (x % y != 0);
}

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

namespace khiin::win32 {

std::wstring kCandidateWindowClassName = L"CandidateWindow";

GUID kCandidateWindowGuid // 829893fa-728d-11ec-8c6e-e0d46491b35a
    = {0x829893fa, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

CandidateWindow::CandidateWindow(HWND parent) : m_hwnd_parent(parent) {}

LRESULT CALLBACK CandidateWindow::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

void CandidateWindow::SetCandidates(messages::CandidateList *candidate_list) {
    D(__FUNCTIONW__);
    m_candidate_list = candidate_list;
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
    if (!m_dwfactory || !m_candidate_list) {
        return;
    }

    EnsureTextFormat();

    candidate_layout_matrix.clear();
    m_col_widths.clear();
    auto &candidates = m_candidate_list->candidates();
    auto total_items_avail = candidates.size();

    if (total_items_avail == 0) {
        return;
    }

    auto min_col_width = (display_mode == DisplayMode::Expanded) ? min_col_width_expanded : min_col_width_single;
    auto max_cols_per_page = (display_mode == DisplayMode::Expanded) ? n_cols_expanded : 1;
    auto max_items_per_col = display_mode == DisplayMode::Short ? short_col_size : long_col_size;
    auto max_items_per_page = (display_mode == DisplayMode::Expanded ? expanded_n_cols : 1) * max_items_per_col;

    auto item_start_idx = max_items_per_page * page_idx;
    auto item_end_idx = min(total_items_avail, max_items_per_page * (page_idx + 1));
    auto n_cols = divide_ceil(item_end_idx - item_start_idx, max_items_per_col);

    auto page_idx = 0;
    auto page_width = 0.0f;
    auto longest_col = 0;
    auto row_height = 0.0f;

    for (auto col_idx = 0; col_idx < n_cols; ++col_idx) {
        auto column = std::vector<winrt::com_ptr<IDWriteTextLayout>>();
        auto column_width = 0.0f;
        auto n_items = min(item_end_idx - item_start_idx, max_items_per_col);
        if (longest_col == 0) {
            longest_col = n_items;
        }

        for (auto row_idx = 0; row_idx < n_items; ++row_idx) {
            auto &item = candidates.at(item_start_idx + row_idx);
            auto &item_value = Utils::Widen(item.value());
            auto item_layout = winrt::com_ptr<IDWriteTextLayout>();
            D(item_value, " (", item_value.size(), ")");
            winrt::check_hresult(m_dwfactory->CreateTextLayout(
                item_value.c_str(), static_cast<UINT32>(item_value.size() + 1), m_textformat.get(),
                static_cast<float>(m_max_width), row_height, item_layout.put()));
            DWRITE_TEXT_METRICS metrics;
            winrt::check_hresult(item_layout->GetMetrics(&metrics));
            column_width = std::max(column_width, metrics.width);
            column.push_back(std::move(item_layout));
            row_height = std::max(row_height, metrics.height);
        }
        item_start_idx += n_items;
        column_width = std::max(column_width, static_cast<float>(min_col_width)) + qs_col_width;
        if (column_width > (min_col_width + qs_col_width - padding)) {
            column_width += padding;
        }
        D("Col width: ", column_width);
        m_col_widths.push_back(column_width);
        page_width += column_width;
        candidate_layout_matrix.push_back(std::move(column));
    }

    row_height += padding;
    m_row_height = row_height;
    auto page_height = longest_col * row_height;
    D("page_height: ", page_height, " (", page_height * m_scale, ")");
    D("page_width: ", page_width, " (", page_width * m_scale, ")");
    ::SetWindowPos(m_hwnd, NULL, 0, 0, static_cast<int>(page_width * m_scale), static_cast<int>(page_height * m_scale),
                   SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    ::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CandidateWindow::OnDpiChanged(WORD dpi, RECT *pNewSize) {
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
    auto origin = D2D1::Point2F(qs_col_width, padding/2);
    D("origin: ", qs_col_width, ",", padding);

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
            origin.y += m_row_height;
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

} // namespace khiin::win32
