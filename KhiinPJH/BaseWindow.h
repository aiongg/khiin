#pragma once

#include "pch.h"

namespace Khiin {

extern HMODULE g_moduleHandle;

class WindowSetup {
  public:
    static void OnDllProcessAttach(HMODULE module);
    static void OnDllProcessDetach(HMODULE module);
};

template <typename DerivedWindow>
class BaseWindow {
  public:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        DerivedWindow *self = NULL;

        if (uMsg == WM_NCCREATE) {
            D("WM_NCCREATE");
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            self = static_cast<DerivedWindow *>(lpcs->lpCreateParams);
            self->m_hwnd = hwnd;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        } else {
            self = reinterpret_cast<DerivedWindow *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (self) {
            return self->WndProc(uMsg, wParam, lParam);
        } else {
            return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    BaseWindow() = default;
    ~BaseWindow() {
        if (m_hwnd != NULL) {
            ::DestroyWindow(m_hwnd);
        }
    }

    HWND hwnd() const {
        return m_hwnd;
    }

  protected:
    virtual std::wstring &class_name() const = 0;
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    HWND m_hwnd = NULL;

    bool Create_(PCWSTR lpWindowName, // clang-format off
                 DWORD dwStyle,
                 DWORD dwExStyle = 0,
                 int x = CW_USEDEFAULT,
                 int y = CW_USEDEFAULT,
                 int nWidth = CW_USEDEFAULT,
                 int nHeight = CW_USEDEFAULT,
                 HWND hWndParent = 0,
                 HMENU hMenu = 0) { // clang-format on

        auto registered = RegisterIfNotExists();

        if (!registered) {
            return false;
        }

        auto previous_dpi_awareness = ::SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        m_hwnd = ::CreateWindowEx(dwExStyle, // clang-format off
                                 class_name().data(),
                                 lpWindowName,
                                 dwStyle,
                                 x, y, nWidth, nHeight,
                                 hWndParent,
                                 hMenu,
                                 g_moduleHandle,
                                 this); // clang-format on
        ::SetThreadDpiAwarenessContext(previous_dpi_awareness);

        if (!m_hwnd) {
            D("CreateWindowEx(...) Failed: ", ::GetLastError());
            return false;
        }

        return true;
    }

    void Destroy_() {
        if (m_hwnd) {
            ::DestroyWindow(m_hwnd);
        }
    }

  private:
    bool RegisterIfNotExists() {
        WNDCLASSEX wc = {0};

        if (::GetClassInfoEx(g_moduleHandle, class_name().data(), &wc)) {
            return TRUE;
        }

        wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_IME;
        wc.lpfnWndProc = DerivedWindow::StaticWndProc;
        wc.cbClsExtra = 0;
        wc.hInstance = g_moduleHandle;
        wc.lpszClassName = class_name().data();
        wc.hIcon = NULL;
        wc.hIconSm = NULL;
        wc.hCursor = NULL;
        wc.lpszMenuName = NULL;
        wc.hbrBackground = (HBRUSH)::GetStockObject(NULL_BRUSH);

        if (!::RegisterClassEx(&wc)) {
            D("RegisterClassEx(&wc) Failed: ", ::GetLastError());
            return false;
        }

        return true;
    }
};

} // namespace Khiin
