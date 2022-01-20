#include "pch.h"

#include "CandidateWindow.h"

namespace Khiin {

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
    case WM_CREATE: {
        D("WM_CREATE");
        try {
            Initialize();
            return 0;
        } catch (...) {
            return -1;
        }
    }
    case WM_DPICHANGED: {
        D("WM_DPICHANGED");
        OnDpiChanged(HIWORD(wParam), (RECT *)lParam);
        return 0;
    }
    case WM_NCPAINT: {
        D("WM_NCPAINT");
        break;
    }
    case WM_PAINT: {
        D("WM_PAINT");
        Render();
        return 0;
    }
    case WM_NCCALCSIZE: {
        D("WM_NCCALCSIZE");
        break;
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

void CandidateWindow::Create() {
    D(__FUNCTIONW__);
    Create_(NULL, // clang-format off
            WS_BORDER | WS_POPUP,
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            0, 0, 100, 100,
            m_hwnd_parent,
            NULL); // clang-format on
}

void CandidateWindow::Destroy() {
    Destroy_();
}

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
    auto left = text_rect_px.left;
    auto top = text_rect_px.bottom + padding;
    ::SetWindowPos(m_hwnd, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CandidateWindow::Initialize() {
    D(__FUNCTIONW__);
    m_d2factory = CreateD2D1Factory();
    m_dwfactory = CreateDwriteFactory();
    m_dpi = ::GetDpiForWindow(m_hwnd);
    m_scale = static_cast<float>(m_dpi) / USER_DEFAULT_SCREEN_DPI;
}

void CandidateWindow::EnsureRenderTarget() {
    D(__FUNCTIONW__);
    if (m_d2factory && m_hwnd && !m_target) {
        m_target = CreateRenderTarget(m_d2factory, m_hwnd);
        m_target->SetDpi(m_dpi, m_dpi);
    }
}

void CandidateWindow::EnsureTextFormat() {
    D(__FUNCTIONW__);
    if (!m_textformat) {
        winrt::check_hresult(m_dwfactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR,
                                                           DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                           font_size, L"en-us", m_textformat.put()));
        winrt::check_hresult(m_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
        winrt::check_hresult(m_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
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
        winrt::check_hresult(m_dwfactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR,
                                                           DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                           font_size, L"en-us", m_textformat.put()));
        winrt::check_hresult(m_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
        winrt::check_hresult(m_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
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
    candidate_layouts.clear();
}

void CandidateWindow::CalculateLayout() {
    D(__FUNCTIONW__);
    if (!m_dwfactory || !candidates) {
        return;
    }

    EnsureTextFormat();

    candidate_layouts.clear();
    int candidates_shown = static_cast<int>(min(candidates->size(), 10));
    float max_width = 0.0f;

    auto it = candidates->cbegin();
    auto end = it + candidates_shown;

    while (it != end) {
        auto &cand = *it;
        D(cand);
        auto layout = winrt::com_ptr<IDWriteTextLayout>();
        winrt::check_hresult(m_dwfactory->CreateTextLayout(cand.data(), static_cast<UINT>(cand.size()),
                                                           m_textformat.get(), 0, 0, layout.put()));
        float min_width;
        layout->DetermineMinWidth(&min_width);
        max_width = max(max_width, min_width);
        candidate_layouts.push_back(std::move(layout));
        ++it;
    }

    auto width = static_cast<int>((max_width + padding * 2.0f) * m_scale);
    auto height = static_cast<int>(candidates_shown * row_height * m_scale);
    D("W", width, "H", height);
    ::SetWindowPos(m_hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CandidateWindow::OnDpiChanged(WORD dpi, RECT *pNewSize) {
    D(__FUNCTIONW__);
    if (m_target) {
        m_target->SetDpi(dpi, dpi);
    }
    m_scale = static_cast<float>(dpi) / USER_DEFAULT_SCREEN_DPI;
    auto width = pNewSize->right - pNewSize->left;
    auto height = pNewSize->bottom - pNewSize->top;
    D("W", width, "H", height);
    ::SetWindowPos(m_hwnd, NULL, pNewSize->left, pNewSize->top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}

void CandidateWindow::OnResize(unsigned int width, unsigned int height) {
    D(__FUNCTIONW__);
    if (m_target) {
        m_target->Resize(D2D1_SIZE_U{width, height});
    }
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
    D2D1_POINT_2F origin = {padding, padding * 2};

    for (auto &layout : candidate_layouts) {
        m_target->DrawTextLayout(origin, layout.get(), m_brush.get());
        origin.y += row_height;
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
