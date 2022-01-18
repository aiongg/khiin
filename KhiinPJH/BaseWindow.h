#pragma once

#include "pch.h"

namespace Khiin {

extern HMODULE g_moduleHandle;

class WindowSetup {
  public:
    static void OnDllProcessAttach(HMODULE module);
};

template <typename Derived>
class BaseWindow {
  public:
    static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Derived *self = NULL;

        if (uMsg == WM_NCCREATE) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            self = static_cast<Derived *>(lpcs->lpCreateParams);
            self->hwnd_ = hwnd;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        } else {
            self = reinterpret_cast<Derived *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (self) {
            return self->wndProc(uMsg, wParam, lParam);
        } else {
            return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    BOOL create(PCWSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle = 0, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT,
                int nWidth = CW_USEDEFAULT, int nHeight = CW_USEDEFAULT, HWND hWndParent = 0, HMENU hMenu = 0) {
        WNDCLASSEX wc = {0};

        wc.lpfnWndProc = Derived::staticWndProc;
        wc.hInstance = WindowSetup::getDllModule();
        wc.lpszClassName = className();

        ::RegisterClassEx(&wc);

        hwnd_ = CreateWindowEx(dwExStyle, className(), lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu,
                               WindowSetup::getDllModule(), this);

        return (hwnd_ ? TRUE : FALSE);
    }

    HWND hwnd() const {
        return hwnd_;
    }

  protected:
    virtual PCWSTR className() const = 0;
    virtual LRESULT wndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    HWND hwnd_ = NULL;
};

} // namespace Khiin
