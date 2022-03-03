#pragma once

namespace khiin::win32::settings {

struct PropSheetPage : PROPSHEETPAGE {
    intptr_t self;
};

class PropSheet {
  public:
    static LRESULT CALLBACK StaticDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    PropSheet(HINSTANCE instance, int template_id);
    PropSheetPage *psp();

  protected:
    virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void Initialize();
    virtual void Cancel();
    virtual void Finalize();
    virtual void OnChange();

    void SetHwnd(HWND hwnd);

    PropSheetPage m_psp = {};
    HINSTANCE m_module = NULL;
    HWND m_hwnd = NULL;
    int m_template_id = 0;
};

} // namespace khiin::win32::settings
