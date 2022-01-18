#include "pch.h"

#include "CandidateWindow.h"

namespace Khiin {

static HMODULE g_moduleHandle;

bool CandidateWindow::OnDllProcessAttach(HMODULE module) {
    g_moduleHandle = module;

    WNDCLASSEXW wnd{};
    wnd.cbSize = sizeof(WNDCLASSEXW);
    wnd.style = CS_DROPSHADOW | CS_HREDRAW | CS_VREDRAW | CS_IME;
    wnd.lpfnWndProc = CandidateWindow::staticWndProc;
    wnd.cbClsExtra = 0;
    wnd.hInstance = g_moduleHandle;
    wnd.lpszClassName = CandidateWindow::className.data();
    wnd.hIcon = NULL;
    wnd.hIconSm = NULL;
    wnd.hCursor = NULL;
    wnd.lpszMenuName = NULL;
    wnd.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);

    if (!::RegisterClassExW(&wnd)) {
        return false;
    }

    return true;
}

LRESULT WINAPI CandidateWindow::staticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    D(__FUNCTIONW__);

    CandidateWindow *self = nullptr;

    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        self = static_cast<CandidateWindow *>(lpcs->lpCreateParams);
        self->hwnd_ = hwnd;
        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<CandidateWindow *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->wndProc(uMsg, wParam, lParam);
    }

    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

CandidateWindow::CandidateWindow(HWND parent) : hwndParent_(parent) {}

CandidateWindow::~CandidateWindow() {
    D(__FUNCTIONW__);
    if (hwnd_ != NULL) {
        ::DestroyWindow(hwnd_);
        hwnd_ = NULL;
    }
}

LRESULT __stdcall CandidateWindow::wndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_PAINT) {
        onPaint();
        return 0;
    }

    return ::DefWindowProc(hwnd_, uMsg, wParam, lParam);
}

void CandidateWindow::create() {
    ::CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, className.data(), NULL,
                     WS_BORDER | WS_POPUP, 0, 0, 100, 100, hwndParent_, NULL, g_moduleHandle, this);
    if (hwnd_ == NULL) {
        D("CreateWindowEx failed: ", ::GetLastError());
    }
}

void CandidateWindow::destroy() {
    D(__FUNCTIONW__);
    if (hwnd_ != NULL) {
        ::DestroyWindow(hwnd_);
        hwnd_ = NULL;
    }
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
    //hr = CreateGraphicsResources();
}

} // namespace Khiin
