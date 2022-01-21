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
    enum class DisplayMode { Short, Long, Expanded };

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
    D2D1::ColorF bg_selected_color = D2D1::ColorF(D2D1::ColorF::LightSkyBlue);
    D2D1::ColorF bg_color = D2D1::ColorF(0.97f, 0.97f, 0.97f);

#pragma warning(push)
#pragma warning(disable : 26812)
    DWM_WINDOW_CORNER_PREFERENCE m_border_radius = DWMWCP_ROUND;
#pragma warning(pop)
    RECT m_border_thickness{};

    unsigned int m_max_width = 0;
    unsigned int m_max_height = 0;
    unsigned int m_dpi_parent = USER_DEFAULT_SCREEN_DPI;
    unsigned int m_dpi = USER_DEFAULT_SCREEN_DPI;
    float m_scale = 1.0f;
    float padding = 16.0f;
    float padding_sm = 4.0f;
    float font_size = 16.0f;
    float row_height = font_size + padding;
    unsigned int min_col_width_single = 160;
    unsigned int min_col_width_expanded = 80;
    unsigned int qs_col_width = 32;
    // unsigned int n_cols = 1;
    // unsigned int n_rows = 9;
    unsigned int n_cols_expanded = 4;
    unsigned int qs_col = 0;
    unsigned int page_idx = 0;
    int selected_idx = -1;
    DisplayMode display_mode = DisplayMode::Expanded;
    unsigned int short_col_size = 5;
    unsigned int long_col_size = 9;
    unsigned int expanded_n_cols = 4;
    std::vector<unsigned int> m_col_widths{};

    std::wstring candidate = L"ŏ\u0358";
    std::vector<std::wstring> *candidates = nullptr;
    std::vector<std::vector<winrt::com_ptr<IDWriteTextLayout>>> candidate_layout_matrix = {};
};

} // namespace Khiin
