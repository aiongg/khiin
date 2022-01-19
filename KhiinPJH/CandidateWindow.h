#pragma once

//#include "common.h"

#include "BaseWindow.h"

namespace Khiin {

extern std::wstring CandidateWindow_ClassName;
extern GUID CandidateWindow_GUID;

class CandidateWindow : public BaseWindow<CandidateWindow> {
  public:
    CandidateWindow(HWND parent);
    CandidateWindow(const CandidateWindow &) = default;
    CandidateWindow &operator=(const CandidateWindow &) = default;
    ~CandidateWindow() = default;

    void create();
    void destroy();

    HRESULT show();
    HRESULT hide();
    bool showing();

    void onPaint();

    virtual std::wstring &className() const override;
    virtual LRESULT WINAPI wndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

  private:
    bool showing_ = false;
    HWND hwndParent_ = NULL;
};

} // namespace Khiin
