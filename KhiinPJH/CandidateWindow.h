#pragma once

//#include "common.h"

#include "BaseWindow.h"

namespace Khiin {

extern std::wstring kCandidateWindowClassName;
extern GUID kCandidateWindowGuid;

class CandidateWindow : public BaseWindow<CandidateWindow> {
  public:
    CandidateWindow(HWND parent);
    CandidateWindow(const CandidateWindow &) = default;
    CandidateWindow &operator=(const CandidateWindow &) = default;
    ~CandidateWindow() = default;

    virtual LRESULT WINAPI WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual std::wstring &class_name() const override;

    void Create();
    void Destroy();
    void Show();
    void Hide();
    bool showing();

    void SetCandidates(std::vector<std::wstring> *candidates);
    void SetScreenCoordinates(RECT text_rect);

  private:
    void OnCreate();
    void OnDisplayChange();
    void OnDpiChanged(WORD dpi, RECT *pSize);
    void OnResize(unsigned int width, unsigned int height);
    void EnsureRenderTarget();
    void EnsureTextFormat();
    void EnsureBrush();
    void CreateGraphicsResources();
    void DiscardGraphicsResources();
    void CalculateLayout();
    void SetBrushColor(D2D1::ColorF);
    void Draw();
    void Render();

    bool m_showing = false;
    HWND m_hwnd_parent = nullptr;

    winrt::com_ptr<ID2D1Factory1> m_d2factory = nullptr;
    winrt::com_ptr<IDWriteFactory3> m_dwfactory = nullptr;
    winrt::com_ptr<ID2D1HwndRenderTarget> m_target = nullptr;
    winrt::com_ptr<ID2D1SolidColorBrush> m_brush = nullptr;
    winrt::com_ptr<IDWriteTextFormat> m_textformat = nullptr;

    D2D1::ColorF text_color = D2D1::ColorF(D2D1::ColorF::Black);
    D2D1::ColorF bg_color = D2D1::ColorF(0.95f, 0.95f, 0.95f);

    RECT m_border_thickness{};
    unsigned int m_max_width = 0;
    unsigned int m_max_height = 0;
    unsigned int m_dpi = USER_DEFAULT_SCREEN_DPI;
    float m_scale = 1.0f;
    float padding = 8.0f;
    float font_size = 16.0f;
    float row_height = font_size + padding;

    std::wstring candidate = L"ŏ\u0358";
    std::vector<std::wstring> *candidates = nullptr;
    std::vector<winrt::com_ptr<IDWriteTextLayout>> candidate_layouts = {};
};

} // namespace Khiin
