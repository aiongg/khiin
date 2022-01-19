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

    void Create();
    void Destroy();
    HRESULT Show();
    HRESULT Hide();
    bool showing();

    void SetCandidates(std::vector<std::wstring> *candidates);

    virtual LRESULT WINAPI WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual std::wstring &class_name() const override;

  private:
    void InitializeDpiScale();
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    HRESULT CreateGraphicsResources();
    void DiscardGraphicsResources();
    HRESULT CalculateLayout();
    void DisableRedraw();
    void EnableRedraw();
    void OnPaint();
    void Resize(int width, int height);
    void SetBrushColor(D2D1::ColorF);

    bool showing_ = false;
    HWND hwnd_parent_ = nullptr;

    winrt::com_ptr<ID2D1Factory> d2d1_factory = nullptr;
    winrt::com_ptr<IDWriteFactory> dwrite_factory = nullptr;
    winrt::com_ptr<ID2D1HwndRenderTarget> render_target = nullptr;
    winrt::com_ptr<ID2D1SolidColorBrush> d2d1_brush = nullptr;
    winrt::com_ptr<IDWriteTextLayout> dwrite_textlayout = nullptr;
    winrt::com_ptr<IDWriteTextFormat> dwrite_textformat = nullptr;
    D2D1_ELLIPSE ellipse{};
    float dpi_scale_x = 1.0f;
    float dpi_scale_y = 1.0f;
    D2D1_RECT_F text_layout{};

    D2D1::ColorF text_color = D2D1::ColorF(D2D1::ColorF::Black);
    D2D1::ColorF bg_color = D2D1::ColorF(D2D1::ColorF::MintCream);

    int client_height = 0;
    int client_width = 0;
    float row_height = 0.0f;

    std::vector<winrt::com_ptr<IDWriteTextLayout>> candidate_layouts = {};

    float font_size = 24.0f;
    float padding = 8.0f;

    std::wstring candidate = L"ŏ\u0358";
    std::vector<std::wstring> *candidates = nullptr;
};

} // namespace Khiin
