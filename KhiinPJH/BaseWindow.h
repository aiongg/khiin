#pragma once

#include "pch.h"

#define MAKEINTATOM(i) (LPTSTR)((ULONG_PTR)((WORD)(i)))

namespace Khiin {

extern HMODULE g_moduleHandle;

class WindowSetup {
  public:
    static void OnDllProcessAttach(HMODULE module);
};

template <typename DerivedWindow>
class BaseWindow {
  public:
    static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        DerivedWindow *self = NULL;

        if (uMsg == WM_NCCREATE) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            self = static_cast<DerivedWindow *>(lpcs->lpCreateParams);
            self->hwnd_ = hwnd;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        } else {
            self = reinterpret_cast<DerivedWindow *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (self) {
            return self->wndProc(uMsg, wParam, lParam);
        } else {
            return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    BaseWindow() = default;
    ~BaseWindow() {
        if (hwnd_ != NULL) {
            ::DestroyWindow(hwnd_);
        }
    }

    HWND hwnd() const {
        return hwnd_;
    }

  protected:
    virtual std::wstring &className() const = 0;
    virtual LRESULT wndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    HWND hwnd_ = NULL;

    bool create_(PCWSTR lpWindowName, // clang-format off
                 DWORD dwStyle,
                 DWORD dwExStyle = 0,
                 int x = CW_USEDEFAULT,
                 int y = CW_USEDEFAULT,
                 int nWidth = CW_USEDEFAULT,
                 int nHeight = CW_USEDEFAULT,
                 HWND hWndParent = 0,
                 HMENU hMenu = 0) { // clang-format on

        auto registered = registerIfNotExists_();

        if (!registered) {
            return false;
        }

        hwnd_ = ::CreateWindowEx(dwExStyle, // clang-format off
                                 className().data(),
                                 lpWindowName,
                                 dwStyle,
                                 x, y, nWidth, nHeight,
                                 hWndParent,
                                 hMenu,
                                 g_moduleHandle,
                                 this); // clang-format on

        if (hwnd_ == NULL) {
            D("CreateWindowEx(...) Failed: ", ::GetLastError());
            return false;
        }

        return true;
    }

    void destroy_() {
        if (hwnd_ != NULL) {
            ::DestroyWindow(hwnd_);
        }
    }

  private:
    bool registerIfNotExists_() {
        WNDCLASSEX wc = {0};

        if (::GetClassInfoEx(g_moduleHandle, className().data(), &wc)) {
            return TRUE;
        }

        wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_DROPSHADOW | CS_HREDRAW | CS_VREDRAW | CS_IME;
        wc.lpfnWndProc = DerivedWindow::staticWndProc;
        wc.cbClsExtra = 0;
        wc.hInstance = g_moduleHandle;
        wc.lpszClassName = className().data();
        wc.hIcon = NULL;
        wc.hIconSm = NULL;
        wc.hCursor = NULL;
        wc.lpszMenuName = NULL;
        wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);

        if (!::RegisterClassEx(&wc)) {
            D("RegisterClassEx(&wc) Failed: ", ::GetLastError());
            return false;
        }

        return true;
    }
};

} // namespace Khiin
