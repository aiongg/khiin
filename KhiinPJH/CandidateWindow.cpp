#include "pch.h"

#include "CandidateWindow.h"

namespace Khiin {

static HMODULE g_moduleHandle;

LRESULT WINAPI CandidateWindow::windowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    return ::DefWindowProc(windowHandle, message, wParam, lParam);
}

bool CandidateWindow::OnDllProcessAttach(HMODULE module) {
    g_moduleHandle = module;

    WNDCLASSEXW wnd{};
    wnd.cbSize = sizeof(WNDCLASSEXW);
    wnd.style = CS_IME;
    wnd.lpfnWndProc = windowProcedure;
    wnd.cbClsExtra = 0;
    wnd.hInstance = g_moduleHandle;
    wnd.lpszClassName = CandidateWindow::className.data();
    wnd.hIcon = NULL;
    wnd.hIconSm = NULL;
    wnd.hCursor = NULL;
    wnd.lpszMenuName = NULL;
    wnd.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH + 1);

    if (!::RegisterClassExW(&wnd)) {
        return false;
    }

    return true;
}

CandidateWindow::~CandidateWindow() {
    destroy();
}

HRESULT CandidateWindow::create(HWND parentWindowHandle) {
    windowHandle = ::CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, className.data(), NULL, WS_BORDER | WS_POPUP, 0,
                                    0, 100, 100, parentWindowHandle, NULL, g_moduleHandle, NULL);
    return S_OK;
}

HRESULT CandidateWindow::show() {
    ::ShowWindow(windowHandle, SW_SHOWNORMAL);
    showing_ = true;
    return S_OK;
}

HRESULT CandidateWindow::hide() {
    ::ShowWindow(windowHandle, SW_HIDE);
    showing_ = false;
    return E_NOTIMPL;
}

HRESULT CandidateWindow::destroy() {
    if (windowHandle) {
        ::DestroyWindow(windowHandle);
        windowHandle = NULL;
    }
    return S_OK;
}

} // namespace Khiin
