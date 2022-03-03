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
            m_candidate_window->SetDisplaySize(config->appearance().size());
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
        *pdwFlags = TF_CLUIE_STRING | TF_CLUIE_COUNT | TF_CLUIE_CURRENTPAGE | TF_CLUIE_PAGEINDEX | TF_CLUIE_SELECTION;
        return S_OK;
    }

    virtual STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppdim) override {
        if (ppdim == nullptr) {
            return E_INVALIDARG;
        }
        m_context->GetDocumentMgr(ppdim);
        return S_OK;
    }

    virtual STDMETHODIMP GetCount(UINT *puCount) override {
        if (puCount == nullptr) {
            return E_INVALIDARG;
        }
        *puCount = m_candidate_list.candidates_size();
        return S_OK;
    }

    virtual STDMETHODIMP GetSelection(UINT *puIndex) override {
        if (puIndex == nullptr) {
            return E_INVALIDARG;
        }
        *puIndex = m_candidate_list.focused();
        return S_OK;
    }

    virtual STDMETHODIMP GetString(UINT uIndex, BSTR *pstr) override {
        if (pstr == nullptr) {
            return E_INVALIDARG;
        }
        if (uIndex >= m_candidate_list.candidates_size()) {
            return E_FAIL;
        }
        auto cand_str = m_candidate_list.candidates().at(uIndex).value();
        auto cand_wstr = Utils::Widen(cand_str);
        BSTR cand_bstr = ::SysAllocStringLen(cand_wstr.data(), cand_wstr.size());
        *pstr = cand_bstr;
        return S_OK;
    }

    virtual STDMETHODIMP GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt) override {
        if (puPageCnt == nullptr) {
            return E_INVALIDARG;
        }
        *puPageCnt = m_pager->PageCount();

        if (pIndex == nullptr) {
            return S_OK;
        }

        if (uSize < *puPageCnt) {
            return E_NOT_SUFFICIENT_BUFFER;
        }

        for (size_t i = 0; i < *puPageCnt; ++i) {
            pIndex[i] = i * m_pager->MaxPageSize();
        }

        return S_OK;
    }

    virtual STDMETHODIMP SetPageIndex(UINT *pIndex, UINT uPageCnt) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetCurrentPage(UINT *puPage) override {
        if (puPage == nullptr) {
            return E_INVALIDARG;
        }
        *puPage = m_pager->CurrentPageIndex();
        return S_OK;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCandidateListUIElementBehavior
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP SetSelection(UINT nIndex) override {
        if (nIndex >= m_candidate_list.candidates_size()) {
            return E_INVALIDARG;
        }

        OnSelectCandidate(nIndex);
        return S_OK;
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
