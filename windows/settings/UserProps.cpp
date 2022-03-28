#include "pch.h"

#include "UserProps.h"

#include "tip/Config.h"

#include "Strings.h"

namespace khiin::win32::settings {
namespace {
using namespace winrt;

struct FileDialogEvents : implements<FileDialogEvents, IFileDialogEvents> {
    STDMETHODIMP OnFileOk(IFileDialog *pfd) override {
        return E_NOTIMPL;
    }
    STDMETHODIMP OnFolderChanging(IFileDialog *pfd, IShellItem *psiFolder) override {
        return E_NOTIMPL;
    }
    STDMETHODIMP OnFolderChange(IFileDialog *pfd) override {
        return E_NOTIMPL;
    }
    STDMETHODIMP OnSelectionChange(IFileDialog *pfd) override {
        return E_NOTIMPL;
    }
    STDMETHODIMP OnShareViolation(IFileDialog *pfd, IShellItem *psi, FDE_SHAREVIOLATION_RESPONSE *pResponse) override {
        return E_NOTIMPL;
    }
    STDMETHODIMP OnTypeChange(IFileDialog *pfd) override {
        return E_NOTIMPL;
    }
    STDMETHODIMP OnOverwrite(IFileDialog *pfd, IShellItem *psi, FDE_OVERWRITE_RESPONSE *pResponse) override {
        return E_NOTIMPL;
    }
};

constexpr COMDLG_FILTERSPEC kFileTypes[] = {{L"Text Document (*.txt)", L"*.txt"}, {L"All Documents (*.*)", L"*.*"}};

std::wstring SelectUserDictionaryFile(HWND parent) {
    try {
        auto fdlg = com_ptr<IFileDialog>();
        check_hresult(::CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(fdlg.put())));
        auto fdlg_events = make<FileDialogEvents>();
        DWORD cookie{};
        check_hresult(fdlg->Advise(fdlg_events.get(), &cookie));
        DWORD flags{};
        check_hresult(fdlg->GetOptions(&flags));
        check_hresult(fdlg->SetOptions(flags | FOS_FORCEFILESYSTEM));
        check_hresult(fdlg->SetFileTypes(ARRAYSIZE(kFileTypes), kFileTypes));
        check_hresult(fdlg->SetFileTypeIndex(1));
        check_hresult(fdlg->Show(NULL));
        auto sh_item = com_ptr<IShellItem>();
        check_hresult(fdlg->GetResult(sh_item.put()));
        PWSTR file_path = NULL;
        check_hresult(sh_item->GetDisplayName(SIGDN_FILESYSPATH, &file_path));
        auto ret = std::wstring(file_path);
        check_hresult(
            ::TaskDialog(parent, NULL, L"起引 PJH", file_path, NULL, TDCBF_OK_BUTTON, TD_INFORMATION_ICON, NULL));
        ::CoTaskMemFree(file_path);
        check_hresult(fdlg->Unadvise(cookie));
        return ret;
    } catch (...) {
        return std::wstring();
    }
}

auto kControlIds = std::vector<uint32_t>{
    IDL_CHOOSE_USERDICT, IDC_RESET_USERDICT_BTN, IDC_CHOOSE_USEDICT_BTN, IDL_RESET_USERDATA, IDC_RESET_USERDATA_BTN,
};

} // namespace

UserProps::UserProps(Application *app) : PropSheetPage(app) {
    m_string_ids = kControlIds;
}

void UserProps::Initialize() {
    PropSheetPage::Initialize();
}

void UserProps::Finalize() {
    PropSheetPage::Finalize();
}

INT_PTR UserProps::DlgProc(UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case IDC_RESET_USERDATA_BTN:
            HandleClearUserData();
            return 0;
        case IDC_CHOOSE_USEDICT_BTN:
            HandleChooseUserDictionaryFile();
            return 0;
        case IDC_RESET_USERDICT_BTN:
            HandleUnloadUserDictionaryFile();
            return 0;
        }
        break;
    }

    return PropSheetPage::DlgProc(msg, wparam, lparam);
}

void UserProps::HandleClearUserData() {
    try {
        int result = 0;
        check_hresult(::TaskDialog(m_hwnd, NULL, L"起引 PJH", L"Clear user history?",
                                   L"This action cannot be undone. Candidates will be reset back to default order.",
                                   TDCBF_CANCEL_BUTTON | TDCBF_OK_BUTTON, TD_WARNING_ICON, &result));
        if (result == IDOK) {
            Config::ClearUserHistory();
            auto hbtn = ItemById(IDC_RESET_USERDATA_BTN);
            BOOL enable = FALSE;
            ::Button_Enable(hbtn, enable);
            auto label = Strings::T(IDL_RESET_USERDATA_BTN_DONE, Config::GetUiLanguage());
            ::Button_SetText(hbtn, label.c_str());
        }
    } catch (...) {
        // Empty
    }
}

void UserProps::HandleChooseUserDictionaryFile() {
    auto file_path = SelectUserDictionaryFile(m_hwnd);

    if (!file_path.empty()) {
        Config::SetKnownFilePath(KhiinFile::UserDb, file_path);
    }
}

void UserProps::HandleUnloadUserDictionaryFile() {}

} // namespace khiin::win32::settings
