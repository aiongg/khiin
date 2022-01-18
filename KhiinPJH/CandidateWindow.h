#pragma once

//#include "common.h"

namespace Khiin {

class CandidateWindow {
  public:
    static inline const std::wstring className = L"CandidateWindow";
    static inline const GUID guid // 829893fa-728d-11ec-8c6e-e0d46491b35a
        {0x829893fa, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

    static bool OnDllProcessAttach(HMODULE module);
    static LRESULT WINAPI staticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    CandidateWindow(HWND parent);
    CandidateWindow(const CandidateWindow &) = default;
    CandidateWindow &operator=(const CandidateWindow &) = default;
    ~CandidateWindow();
    
    LRESULT WINAPI wndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void create();
    void destroy();

    HRESULT show();
    HRESULT hide();
    bool showing();

    void onPaint();

  private:
    bool showing_ = false;
    HWND hwnd_ = NULL;
    HWND hwndParent_ = NULL;
};

} // namespace Khiin
