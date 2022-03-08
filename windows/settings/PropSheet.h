#pragma once

namespace khiin::win32::settings {

struct PropSheetPage : PROPSHEETPAGE {
    intptr_t self;
};

class Application;

class PropSheet {
  public:
    PropSheet(Application *app);
    static LRESULT CALLBACK StaticDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HPROPSHEETPAGE psp(HMODULE hmod, int template_id, messages::AppConfig *config);
    void Reload();
    HWND hwnd();

  protected:
    virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void Initialize();
    virtual void Cancel();
    virtual void Finalize();
    virtual void OnChange();

    void InitComboBox(uint32_t control_rid, std::vector<uint32_t> const &option_rids, int selected_index);
    void SetHwnd(HWND hwnd);

    PropSheetPage m_psp = {};
    HPROPSHEETPAGE m_hpsp = NULL;
    HMODULE m_module = NULL;
    HWND m_hwnd = NULL;
    uint32_t m_template_id = 0;
    messages::AppConfig *m_config = nullptr;

    std::vector<uint32_t> m_string_ids;
    std::unordered_map<messages::UiLanguage, std::vector<uint32_t>> m_translations;
    Application *m_app = nullptr;
};

} // namespace khiin::win32::settings
