#pragma once

namespace khiin::win32::settings {

struct PropSheetPage : PROPSHEETPAGE {
    intptr_t self;
};

class PropSheet {
  public:
    static LRESULT CALLBACK StaticDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HPROPSHEETPAGE psp(HMODULE hmod, int template_id, messages::AppConfig *config);

  protected:
    virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void Initialize();
    virtual void Cancel();
    virtual void Finalize();
    virtual void OnChange();

    void SetHwnd(HWND hwnd);

    PropSheetPage m_psp = {};
    HPROPSHEETPAGE m_hpsp = NULL;
    HMODULE m_module = NULL;
    HWND m_hwnd = NULL;
    int m_template_id = 0;
    messages::AppConfig *m_config = nullptr;
};

} // namespace khiin::win32::settings
