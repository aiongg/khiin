#pragma once

#include "BaseWindow.h"

namespace khiin::win32 {

class GuiWindow : public BaseWindow<GuiWindow> {
  public:
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    // Subclasses should implement to use the mouse
    virtual void OnMouseMove(uint32_t x, uint32_t y);
    virtual void OnMouseLeave();
    virtual bool OnClick(uint32_t x, uint32_t y);
    
    // Required
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void Render() = 0;
    virtual bool Showing();

    // Load graphics resources
    void OnCreate();
    void OnMonitorSizeChanged();
    void OnResize(uint32_t width, uint32_t height);
    void OnDpiChanged(WORD dpi, RECT *pNewSize);
    void EnsureRenderTarget();

  protected:
    bool m_showing = false;
    winrt::com_ptr<ID2D1Factory1> m_d2d1 = nullptr;
    winrt::com_ptr<IDWriteFactory3> m_dwrite = nullptr;
    winrt::com_ptr<ID2D1DCRenderTarget> m_target = nullptr;

    uint32_t m_max_width = 0;
    uint32_t m_max_height = 0;
    uint32_t m_dpi_parent = USER_DEFAULT_SCREEN_DPI;
    uint32_t m_dpi = USER_DEFAULT_SCREEN_DPI;
    float m_scale = 1.0f;
};

} // namespace khiin::win32