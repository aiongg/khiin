#include "pch.h"

#include "CandidateListUI.h"

#include "CandidatePager.h"
#include "CandidateWindow.h"
#include "Colors.h"
#include "Config.h"
#include "EditSession.h"
#include "TextService.h"
#include "Utils.h"
#include "common.h"

namespace khiin::win32 {
namespace {

using namespace messages;

bool IsArrowKey(int key_code) {
    return key_code == VK_LEFT || key_code == VK_UP || key_code == VK_RIGHT || key_code == VK_DOWN;
}

struct CandidateListUIImpl :
    public winrt::implements<CandidateListUIImpl, ITfCandidateListUIElementBehavior,
                             ITfIntegratableCandidateListUIElement, CandidateListUI>,
    CandidateSelectListener,
    ConfigChangeListener {
    virtual void Initialize(TextService *pTextService) override {
        m_service.copy_from(pTextService);
        m_service->RegisterConfigChangeListener(this);
        m_pager = std::unique_ptr<CandidatePager>(CandidatePager::Create());
    }

    virtual void Uninitialize() override {
        D(__FUNCTIONW__);
        m_service = nullptr;
        m_context = nullptr;
        DestroyCandidateWindow();
        m_candidate_window.reset(nullptr);
        m_pager.reset(nullptr);
    }

    virtual void DestroyCandidateWindow() override {
        D(__FUNCTIONW__);
        if (m_candidate_window && m_candidate_window->hwnd()) {
            ::DestroyWindow(m_candidate_window->hwnd());
        }
    }

    virtual void Update(ITfContext *pContext, EditState edit_state, const messages::CandidateList &candidate_list,
                        RECT text_rect) override {
        D(__FUNCTIONW__);
        m_context.copy_from(pContext);
        m_candidate_list.CopyFrom(candidate_list);
        m_edit_state = edit_state;
        m_text_rect = text_rect;

        if (!m_candidate_window) {
            RegisterCandidateWindow();
        }

        if (m_edit_state == EditState::EDIT_COMPOSING) {
            m_pager->SetCandidateList(&m_candidate_list);
        } else if (m_edit_state == EditState::EDIT_SELECTING) {
            m_pager->SetFocus(m_candidate_list.focused());
        }

        UpdateWindow();
    }

    virtual bool Showing() override {
        return m_candidate_window && m_candidate_window->Showing();
    }

    virtual bool Selecting() override {
        return m_edit_state == EditState::EDIT_SELECTING;
    }

    virtual bool MultiColumn() override {
        return m_candidate_grid.size() > 1;
    }

    virtual int PageCount() override {
        return m_pager->PageCount();
    }

    virtual int MaxQuickSelect() override {
        auto focused_col = m_pager->GetFocusedColumnIndex();
        if (focused_col < m_candidate_grid.size()) {
            return static_cast<int>(m_candidate_grid.at(focused_col).size());
        }
        return 0;
    }

    virtual int QuickSelect(int index) override {
        auto focused_col = m_pager->GetFocusedColumnIndex();

        if (focused_col < m_candidate_grid.size()) {
            auto &col = m_candidate_grid.at(focused_col);
            if (index < col.size()) {
                auto cand = col.at(index);
                return cand->id();
            }
        }

        return -1;
    }

    virtual int RotateNext() override {
        if (m_pager->PageCount() < 2) {
            return -1;
        }

        return m_pager->NextPageCandidateId();
    }

    virtual int RotatePrev() override {
        if (m_pager->PageCount() < 2) {
            return -1;
        }

        return m_pager->PrevPageCandidateId();
    }

    virtual void Show() override {
        if (m_candidate_window) {
            m_candidate_window->Show();
        }
    }

    virtual void Hide() override {
        if (m_candidate_window) {
            m_candidate_window->Hide();
        }
    }

    //+---------------------------------------------------------------------------
    //
    // CandidateSelectListener
    //
    //----------------------------------------------------------------------------

    virtual void OnSelectCandidate(int32_t id) override {
        m_service->OnCandidateSelected(id);
    }

    //+---------------------------------------------------------------------------
    //
    // ConfigChangeListener
    //
    //----------------------------------------------------------------------------

    virtual void OnConfigChanged(messages::AppConfig *config) override {
        D(__FUNCTIONW__);
        if (m_candidate_window) {
            m_candidate_window->SetAppearance(Colors::GetScheme(config));
        }
    }

    //+---------------------------------------------------------------------------
    //
    // ITfUIElement
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP GetDescription(BSTR *pbstrDescription) override {
        TRY_FOR_HRESULT;
        BSTR bstr = ::SysAllocString(kCandidateWindowClassName.data());
        pbstrDescription = &bstr;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetGUID(GUID *pguid) override {
        TRY_FOR_HRESULT;
        *pguid = kCandidateWindowGuid;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP Show(BOOL bShow) override {
        TRY_FOR_HRESULT;
        if (!m_candidate_window) {
            return S_OK;
        }

        if (bShow) {
            Show();
        } else {
            Hide();
        }

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP IsShown(BOOL *pbShow) override {
        D(__FUNCTIONW__);
        TRY_FOR_HRESULT;
        if (!m_candidate_window) {
            *pbShow = FALSE;
            return S_OK;
        }

        *pbShow = Showing() ? TRUE : FALSE;
        CATCH_FOR_HRESULT;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCandidateListUIElement
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP GetUpdatedFlags(DWORD *pdwFlags) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppdim) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetCount(UINT *puCount) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetSelection(UINT *puIndex) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetString(UINT uIndex, BSTR *pstr) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP SetPageIndex(UINT *pIndex, UINT uPageCnt) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetCurrentPage(UINT *puPage) override {
        return E_NOTIMPL;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCandidateListUIElementBehavior
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP SetSelection(UINT nIndex) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP Finalize(void) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP Abort(void) override {
        return E_NOTIMPL;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfIntegratableCandidateListUIElement
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP SetIntegrationStyle(GUID guidIntegrationStyle) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetSelectionStyle(TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP ShowCandidateNumbers(BOOL *pfShow) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP FinalizeExactCompositionString(void) override {
        return E_NOTIMPL;
    }

  private:
    void RegisterCandidateWindow() {
        auto contextView = winrt::com_ptr<ITfContextView>();
        winrt::check_hresult(m_context->GetActiveView(contextView.put()));

        HWND parentWnd;
        winrt::check_hresult(contextView->GetWnd(&parentWnd));

        if (!(parentWnd)) {
            parentWnd = ::GetFocus();
        }

        m_candidate_window = std::unique_ptr<CandidateWindow>(CandidateWindow::Create(parentWnd));
        OnConfigChanged(m_service->config());
        m_candidate_window->RegisterCandidateSelectListener(this);
    }

    void UpdateWindow() {
        m_candidate_grid.clear();
        m_pager->GetPage(m_candidate_grid);
        bool qs_active = m_edit_state == EditState::EDIT_SELECTING;
        auto focused_cand_id = m_pager->GetFocusedCandidateId();
        auto focused_col = m_pager->GetFocusedColumnIndex();
        auto display_mode = m_pager->GetDisplayMode();

        m_candidate_window->SetCandidates(display_mode, &m_candidate_grid, focused_cand_id, focused_col, qs_active,
                                          m_text_rect);
        Show();
    }

    bool DigitKeyIsQuickSelect(char key_char) {
        auto key_int = key_char - '0';
        auto available_qs = m_candidate_grid.at(m_focused_col).size();
        if (key_int > 0 && key_int <= available_qs) {
            return true;
        }
        return false;
    }

    winrt::com_ptr<TextService> m_service = nullptr;
    winrt::com_ptr<ITfContext> m_context = nullptr;
    CandidateList m_candidate_list = {};
    std::unique_ptr<CandidateWindow> m_candidate_window = nullptr;
    std::unique_ptr<CandidatePager> m_pager = nullptr;
    EditState m_edit_state = EditState::EDIT_EMPTY;
    CandidateGrid m_candidate_grid = CandidateGrid();
    size_t m_focused_col = 0;
    RECT m_text_rect = {};
};

} // namespace

HRESULT CandidateListUIFactory::Create(CandidateListUI **ppCandidateListUI) {
    as_self<CandidateListUI>(winrt::make_self<CandidateListUIImpl>()).copy_to(ppCandidateListUI);
    return S_OK;
}

} // namespace khiin::win32
