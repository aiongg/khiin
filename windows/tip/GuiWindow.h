#pragma once

#include "BaseWindow.h"
#include "Colors.h"
#include "Geometry.h"
#include "common.h"

namespace khiin::win32 {

class RenderFactory;

enum class DpiAwarenessContext {
    Gdiscaled,
    Unaware,
    System,
    PerMonitor,
    PerMonitorV2,
};

class GuiWindow : public BaseWindow<GuiWindow> {
  public:
    using Point = geometry::Point;
    GuiWindow();
    ~GuiWindow();
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    // To enable mouse tracking & click handling, subclasses must
    // implement |OnMouseMove| and |OnClick|, as well as call |::SetCapture|
    // before the mouse will be usable, e.g. in |Show|.
    // (Don't forget to call |::ReleaseCapture| in |Hide|.)
    virtual void OnMouseMove(Point pt);
    virtual bool OnClick(Point pt);

    virtual void Show();
    virtual void Hide();
    virtual bool Showing();
    virtual void OnConfigChanged(messages::AppConfig *config);

    // Required
    virtual void Render() = 0;

    // Load graphics resources
    void OnCreate();
    void OnMonitorSizeChanged();
    void OnResize(uint32_t width, uint32_t height);
    void OnDpiChanged(WORD dpi, RECT *pNewSize);
    void EnsureRenderTarget();

  protected:
    bool ClientHitTest(Point const &pt);
    void ClientDp(Point &pt);
    uint32_t EffectiveDpi();
    DpiAwarenessContext DpiAwareness();
    bool DpiAware();
    int ToPx(int value);
    int ToDp(int value);

    bool m_showing = false;
    bool m_tracking_mouse = false;
    uint32_t m_max_width = 0;
    uint32_t m_max_height = 0;
    uint32_t m_dpi_parent = USER_DEFAULT_SCREEN_DPI;
    uint32_t m_dpi = USER_DEFAULT_SCREEN_DPI;
    float m_scale = 1.0f;
    std::unique_ptr<RenderFactory> m_factory = nullptr;
    winrt::com_ptr<ID2D1DCRenderTarget> m_target = nullptr;
    messages::AppConfig *m_config = nullptr;
    ColorScheme m_colors;
    messages::UiLanguage m_language = messages::UIL_TAI_HANLO;
};

} // namespace khiin::win32