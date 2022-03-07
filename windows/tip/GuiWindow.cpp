#include "pch.h"

#include "GuiWindow.h"

#include "Graphics.h"

namespace khiin::win32 {

#pragma warning(push)
#pragma warning(disable : 26812)
void SetRoundedCorners(HWND hwnd, DWM_WINDOW_CORNER_PREFERENCE pref) {
    ::DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &pref, sizeof(DWM_WINDOW_CORNER_PREFERENCE));
}
#pragma warning(pop)

LRESULT GuiWindow::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_NCCREATE:
        KHIIN_TRACE("WM_NCCREATE");
        SetRoundedCorners(m_hwnd, DWMWCP_ROUND);
        break;
    case WM_CREATE:
        KHIIN_TRACE("WM_CREATE");
        try {
            OnCreate();
            return 0;
        } catch (...) {
            return -1;
        }
    case WM_DISPLAYCHANGE:
        KHIIN_TRACE("WM_DISPLAYCHANGE");
        OnMonitorSizeChanged();
        break;
    case WM_DPICHANGED:
        KHIIN_TRACE("WM_DPICHANGED");
        OnDpiChanged(HIWORD(wParam), (RECT *)lParam);
        return 0;
    case WM_MOUSEACTIVATE:
        KHIIN_TRACE("WM_MOUSEACTIVATE");
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    case WM_MOUSELEAVE:
        KHIIN_TRACE("WM_MOUSELEAVE");
        OnMouseLeave();
        break;
    case WM_LBUTTONDOWN:
        KHIIN_DEBUG("LBUTTONDOWN");
        if (OnClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) {
            return 0;
        }
        break;
    case WM_PAINT:
        KHIIN_TRACE("WM_PAINT");
        Render();
        return 0;
    case WM_SIZE:
        KHIIN_TRACE("WM_SIZE");
        OnResize(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_WINDOWPOSCHANGING:
        KHIIN_TRACE("WM_WINDOWPOSCHANGING");
        OnMonitorSizeChanged();
        break;
    default:
        KHIIN_TRACE("Unknown WM ({})", uMsg);
        break;
    }

    return ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}


void GuiWindow::OnCreate() {
    m_d2d1 = Graphics::CreateD2D1Factory();
    m_dwrite = Graphics::CreateDwriteFactory();
    m_dpi = ::GetDpiForWindow(m_hwnd);
    m_dpi_parent = ::GetDpiForWindow(::GetParent(m_hwnd));
    m_scale = static_cast<float>(m_dpi / USER_DEFAULT_SCREEN_DPI);
    OnMonitorSizeChanged();
}

void GuiWindow::OnMonitorSizeChanged() {
    auto hmon = ::MonitorFromWindow(::GetParent(m_hwnd), MONITOR_DEFAULTTONEAREST);
    auto info = MONITORINFO();
    info.cbSize = sizeof(MONITORINFO);
    ::GetMonitorInfo(hmon, &info);
    m_max_width = info.rcMonitor.right;
    m_max_height = info.rcMonitor.bottom;
}

void GuiWindow::OnDpiChanged(WORD dpi, RECT *pNewSize) {
    if (m_target) {
        m_target->SetDpi(dpi, dpi);
    }
    m_dpi = dpi;
    m_dpi_parent = ::GetDpiForWindow(::GetParent(m_hwnd));
    m_scale = static_cast<float>(m_dpi_parent) / USER_DEFAULT_SCREEN_DPI;
    auto width = pNewSize->right - pNewSize->left;
    auto height = pNewSize->bottom - pNewSize->top;
    ::SetWindowPos(m_hwnd, NULL, pNewSize->left, pNewSize->top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}

void GuiWindow::OnResize(uint32_t width, uint32_t height) {
    KHIIN_TRACE("");
    EnsureRenderTarget();
    if (m_target) {
        //m_target->Resize(D2D1_SIZE_U{width, height});
    }
}

bool GuiWindow::Showing() {
    return m_showing;
}

void GuiWindow::EnsureRenderTarget() {
    if (m_d2d1 && m_hwnd && !m_target) {
        m_target = Graphics::CreateDCRenderTarget(m_d2d1);
        if (m_target) {
            m_target->SetDpi(static_cast<float>(m_dpi), static_cast<float>(m_dpi));
        }
    }
}

void GuiWindow::OnMouseMove(uint32_t x, uint32_t y) {
    // override
}

void GuiWindow::OnMouseLeave() {
    // override
}

bool GuiWindow::OnClick(uint32_t x, uint32_t y) {
    // override
    return false;
}

} // namespace khiin::win32
