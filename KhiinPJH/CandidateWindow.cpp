#include "pch.h"

#include "CandidateWindow.h"

namespace Khiin {

std::wstring kCandidateWindowClassName = L"CandidateWindow";
GUID kCandidateWindowGuid // 829893fa-728d-11ec-8c6e-e0d46491b35a
    = {0x829893fa, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

LRESULT __stdcall CandidateWindow::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    D(__FUNCTIONW__, " (", uMsg, ")");

    switch (uMsg) {
    case WM_CREATE: {
        return SUCCEEDED(CreateDeviceIndependentResources()) ? 0 : -1;
    }
    case WM_PAINT: {
        OnPaint();
        return 0;
    }
    //case WM_DPICHANGED: {
    //    DiscardGraphicsResources();
    //    return 0;
    //}
    //case WM_SIZE: {
    //    DiscardGraphicsResources();
    //    return 0;
    //}
    }

    return ::DefWindowProc(hwnd_, uMsg, wParam, lParam);
}

CandidateWindow::CandidateWindow(HWND parent) : hwnd_parent_(parent) {}

void CandidateWindow::Create() {
    Create_(NULL, // clang-format off
            WS_BORDER | WS_POPUP,
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            0, 0, 100, 100,
            hwnd_parent_,
            NULL); // clang-format on
}

void CandidateWindow::Destroy() {
    Destroy_();
}

HRESULT CandidateWindow::Show() {
    if (showing_) {
        return S_OK;
    }

    ::ShowWindow(hwnd_, SW_SHOWNA);
    showing_ = true;
    return S_OK;
}

HRESULT CandidateWindow::Hide() {
    if (!showing_) {
        return S_OK;
    }

    ::ShowWindow(hwnd_, SW_HIDE);
    showing_ = false;
    return S_OK;
}

bool CandidateWindow::showing() {
    return showing_;
}

void CandidateWindow::SetCandidates(std::vector<std::wstring> *candidates) {
    this->candidates = candidates;
    LayoutAndRedraw();
}

void CandidateWindow::SetScreenCoordinates(RECT text_rect_px) {
    auto left = text_rect_px.left;
    auto top = text_rect_px.bottom + padding * dpi_scale;
    Reposition(left, top);
}

void CandidateWindow::InitializeDpiScale() {
    auto dpi = ::GetDpiForWindow(hwnd_);
    if (current_dpi != dpi) {
        DiscardGraphicsResources();
    }
    current_dpi = dpi;
    dpi_scale = dpi / 96.0f;
}

HRESULT CandidateWindow::CreateDeviceIndependentResources() {
    auto hr = E_FAIL;

    if (!d2d1_factory) {
        hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2d1_factory.put());
        CHECK_RETURN_HRESULT(hr);
    }

    if (!dwrite_factory) {
        hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                                   reinterpret_cast<IUnknown **>(dwrite_factory.put()));
        CHECK_RETURN_HRESULT(hr);
    }

    if (!dwrite_textformat) {
        hr = dwrite_factory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
                                              DWRITE_FONT_STRETCH_NORMAL, font_size, L"en-us", dwrite_textformat.put());
        CHECK_RETURN_HRESULT(hr);

        hr = dwrite_textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        CHECK_RETURN_HRESULT(hr);

        hr = dwrite_textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        CHECK_RETURN_HRESULT(hr);
    }

    return S_OK;
}

HRESULT CandidateWindow::CreateDeviceResources() {
    auto hr = E_FAIL;

    RECT rc;
    ::GetClientRect(hwnd_, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

    if (!render_target) {
        hr = d2d1_factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                  D2D1::HwndRenderTargetProperties(hwnd_, size), render_target.put());
        CHECK_RETURN_HRESULT(hr);
    }

    return S_OK;
}

HRESULT CandidateWindow::CreateGraphicsResources() {
    auto hr = E_FAIL;

    InitializeDpiScale();
    hr = CreateDeviceIndependentResources();
    CHECK_RETURN_HRESULT(hr);
    hr = CreateDeviceResources();
    CHECK_RETURN_HRESULT(hr);
    hr = CalculateLayout();
    CHECK_RETURN_HRESULT(hr);

    if (!d2d1_brush) {
        hr = render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), d2d1_brush.put());
        CHECK_RETURN_HRESULT(hr);
    }


    return S_OK;
}

void CandidateWindow::DiscardGraphicsResources() {
    render_target = nullptr;
    d2d1_brush = nullptr;
    dwrite_textformat = nullptr;
    candidate_layouts.clear();
}

void CandidateWindow::LayoutAndRedraw() {
    if (!dwrite_factory || !candidates) {
        return ;
    }

    DisableRedraw();
    CalculateLayout();
    Resize(client_width, client_height);
    EnableRedraw();

    return ;
}

HRESULT CandidateWindow::CalculateLayout() {
    if (!dwrite_factory || !candidates) {
        return S_FALSE;
    }

    candidate_layouts.clear();
    int candidates_shown = static_cast<int>(min(candidates->size(), 10));
    float max_width = 0.0f;

    auto it = candidates->cbegin();
    auto end = it + candidates_shown;

    while (it != end) {
        auto &cand = *it;
        auto layout = winrt::com_ptr<IDWriteTextLayout>();
        auto hr = dwrite_factory->CreateTextLayout(cand.data(), static_cast<UINT>(cand.size()), dwrite_textformat.get(),
                                                   0, 0, layout.put());
        CHECK_RETURN_HRESULT(hr);
        float min_width;
        layout->DetermineMinWidth(&min_width);
        max_width = max(max_width, min_width);
        candidate_layouts.push_back(std::move(layout));
        ++it;
    }

    client_width = static_cast<int>(max_width + padding * 2.0f);
    client_height = static_cast<int>(candidates_shown * row_height);

    return S_OK;
}

void CandidateWindow::DisableRedraw() {
    ::SendMessage(hwnd_, WM_SETREDRAW, FALSE, FALSE);
}

void CandidateWindow::EnableRedraw() {
    ::SendMessage(hwnd_, WM_SETREDRAW, TRUE, FALSE);
    ::RedrawWindow(hwnd_, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void CandidateWindow::OnPaint() {
    auto hr = CreateGraphicsResources();
    if (FAILED(hr)) {
        return;
    }

    PAINTSTRUCT ps;
    ::BeginPaint(hwnd_, &ps);

    render_target->BeginDraw();
    render_target->Clear(bg_color);

    SetBrushColor(text_color);
    D2D1_POINT_2F origin = {padding, padding * 2};

    for (auto &layout : candidate_layouts) {
        render_target->DrawTextLayout(origin, layout.get(), d2d1_brush.get());
        origin.y += row_height;
    }

    hr = render_target->EndDraw();
    if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
        DiscardGraphicsResources();
    }
    ::EndPaint(hwnd_, &ps);
}

void CandidateWindow::OnDpiChanged(WORD dpi, RECT *pSize) {
    if (!render_target) {
        return;
    }
    D(__FUNCTIONW__, dpi);
    // auto size = D2D1::SizeU(pSize->right, pSize->bottom);
    // render_target->SetDpi(dpi, dpi);
    // render_target->Resize(size);
    // client_left = pSize->left;
    // client_top = pSize->top;
    // client_width = pSize->right - pSize->left;
    // client_height = pSize->bottom - pSize->top;
    // ResetWindowPosition();
}

void CandidateWindow::Resize(int width, int height) {
    client_width = width;
    client_height = height;
    ResetWindowPosition();
}

void CandidateWindow::Reposition(int left, int top) {
    client_left = left;
    client_top = top;
    ResetWindowPosition();
}

void CandidateWindow::ResetWindowPosition() {
    ::SetWindowPos(hwnd_, HWND_TOP, client_left, client_top, client_width * dpi_scale,
                   client_height * dpi_scale, SWP_NOZORDER | SWP_NOACTIVATE);
}

void CandidateWindow::SetBrushColor(D2D1::ColorF color) {
    d2d1_brush->SetColor(D2D1::ColorF(color));
}

std::wstring &CandidateWindow::class_name() const {
    return kCandidateWindowClassName;
}

} // namespace Khiin
