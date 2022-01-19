#include "pch.h"

#include "CandidateWindow.h"

namespace Khiin {

std::wstring CandidateWindow_ClassName = L"CandidateWindow";
GUID CandidateWindow_GUID // 829893fa-728d-11ec-8c6e-e0d46491b35a
    = {0x829893fa, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

CandidateWindow::CandidateWindow(HWND parent) : hwndParent_(parent) {}

LRESULT __stdcall CandidateWindow::wndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    D(__FUNCTIONW__, " (", uMsg, ")");
    // TODO
    // if (uMsg == WM_PAINT) {
    //    onPaint();
    //    return 0;
    //}

    return ::DefWindowProc(hwnd_, uMsg, wParam, lParam);
}

void CandidateWindow::create() {
    create_(NULL, // clang-format off
            WS_BORDER | WS_POPUP,
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            0, 0, 100, 100,
            hwndParent_,
            NULL); // clang-format on
}

void CandidateWindow::destroy() {
    destroy_();
}

HRESULT CandidateWindow::show() {
    if (showing_) {
        return S_OK;
    }

    ::ShowWindow(hwnd_, SW_SHOWNA);
    showing_ = true;
    return S_OK;
}

HRESULT CandidateWindow::hide() {
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

void CandidateWindow::onPaint() {
    auto hr = E_FAIL;
    // hr = CreateGraphicsResources();
}

std::wstring &CandidateWindow::className() const {
    return CandidateWindow_ClassName;
}

} // namespace Khiin
