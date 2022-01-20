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
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            self = static_cast<DerivedWindow *>(lpcs->lpCreateParams);
            self->hwnd_ = hwnd;
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
        if (hwnd_ != NULL) {
            ::DestroyWindow(hwnd_);
        }
    }

    HWND hwnd() const {
        return hwnd_;
    }

  protected:
    virtual std::wstring &class_name() const = 0;
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    HWND hwnd_ = NULL;

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

    //    auto f = ::SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    //    auto prevHosting = ::SetThreadDpiHostingBehavior(DPI_HOSTING_BEHAVIOR_MIXED);

    //    auto g = ::GetAwarenessFromDpiAwarenessContext(f);
    //    D("awareness: ", g);
    //auto msg = std::wstring(L"UNKNOWN");
    //            if (::AreDpiAwarenessContextsEqual(f, DPI_AWARENESS_CONTEXT_UNAWARE)) {
    //        msg = L"unaware";
    //    } else if (::AreDpiAwarenessContextsEqual(f, DPI_AWARENESS_CONTEXT_SYSTEM_AWARE)) {
    //        msg = L"system";
    //    } else if (::AreDpiAwarenessContextsEqual(f, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) {
    //        msg = L"pm";
    //    } else if (::AreDpiAwarenessContextsEqual(f, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) {
    //        msg = L"pm2";
    //    } else if (::AreDpiAwarenessContextsEqual(f, DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED)) {
    //        msg = L"gdi";
    //    }

    //    D("Prevawareness: ", f, " ", msg);
    //    D("Prevhosting", prevHosting);

        hwnd_ = ::CreateWindowEx(dwExStyle, // clang-format off
                                 class_name().data(),
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

        //::SetThreadDpiHostingBehavior(prevHosting);
        //::SetThreadDpiAwarenessContext(f);

        return true;
    }

    void Destroy_() {
        if (hwnd_ != NULL) {
            ::DestroyWindow(hwnd_);
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
        wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);

        if (!::RegisterClassEx(&wc)) {
            D("RegisterClassEx(&wc) Failed: ", ::GetLastError());
            return false;
        }

        return true;
    }
};

} // namespace Khiin
