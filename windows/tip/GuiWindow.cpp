#include "pch.h"

#include "GuiWindow.h"

#include "RenderFactory.h"

namespace khiin::win32 {
using namespace geometry;
using namespace messages;

#pragma warning(push)
#pragma warning(disable : 26812)
void SetRoundedCorners(HWND hwnd, DWM_WINDOW_CORNER_PREFERENCE pref) {
    ::DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &pref, sizeof(DWM_WINDOW_CORNER_PREFERENCE));
}
#pragma warning(pop)

GuiWindow::GuiWindow() = default;

GuiWindow::~GuiWindow() = default;

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
        OnMouseMove(Point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
        break;
    case WM_MOUSELEAVE:
        KHIIN_TRACE("WM_MOUSELEAVE");
        // OnMouseLeave();
        break;
    case WM_LBUTTONDOWN:
        KHIIN_DEBUG("LBUTTONDOWN");
        if (OnClick(Point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) {
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
    m_factory = RenderFactory::Create();
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
        // m_target->Resize(D2D1_SIZE_U{width, height});
    }
}

void GuiWindow::Show() {
    if (m_showing) {
        return;
    }

    ::ShowWindow(m_hwnd, SW_SHOWNA);
    m_showing = true;

    if (!m_tracking_mouse) {
        ::SetCapture(m_hwnd);
        m_tracking_mouse = true;
    }
}

void GuiWindow::Hide() {
    if (!m_showing) {
        return;
    }

    ::ShowWindow(m_hwnd, SW_HIDE);
    m_showing = false;

    if (m_tracking_mouse) {
        ::ReleaseCapture();
        m_tracking_mouse = false;
    }
}

bool GuiWindow::Showing() {
    return m_showing;
}

void GuiWindow::EnsureRenderTarget() {
    if (m_factory && m_hwnd && !m_target) {
        m_target = m_factory->CreateDCRenderTarget();
        if (m_target) {
            m_target->SetDpi(static_cast<float>(m_dpi), static_cast<float>(m_dpi));
        }
    }
}

void GuiWindow::OnConfigChanged(AppConfig *config) {
    m_config = config;
    m_colors = Colors::GetScheme(config);
    m_language = config->appearance().ui_language();
}

bool GuiWindow::ClientHitTest(Point const &pt) {
    auto rect = RECT{};
    ::GetClientRect(m_hwnd, &rect);
    POINT pt_px{static_cast<long>(pt.x), static_cast<long>(pt.y)};

    return ::PtInRect(&rect, pt_px);
}

void GuiWindow::ClientDp(Point &pt) {
    pt.x = static_cast<int>(pt.x / m_scale);
    pt.y = static_cast<int>(pt.y / m_scale);
}

void GuiWindow::OnMouseMove(Point pt) {
    // override
}

bool GuiWindow::OnClick(Point pt) {
    // override
    return false;
}

} // namespace khiin::win32
