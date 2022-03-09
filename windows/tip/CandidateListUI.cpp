#include "pch.h"

#include "CandidateListUI.h"

#include "proto/proto.h"

#include "CandidatePager.h"
#include "CandidateWindow.h"
#include "Colors.h"
#include "Config.h"
#include "EditSession.h"
#include "TextService.h"
#include "Utils.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;
using namespace proto;

bool IsArrowKey(int key_code) {
    return key_code == VK_LEFT || key_code == VK_UP || key_code == VK_RIGHT || key_code == VK_DOWN;
}

struct CandidateListUIImpl :
    public implements<CandidateListUIImpl, ITfCandidateListUIElementBehavior, CandidateListUI>,
    CandidateSelectListener,
    ConfigChangeListener {
    virtual void Initialize(TextService *pTextService) override {
        KHIIN_TRACE("");
        m_service.copy_from(pTextService);
        m_service->RegisterConfigChangeListener(this);
        m_pager = std::unique_ptr<CandidatePager>(CandidatePager::Create());
        BeginUIElement();
    }

    virtual void Uninitialize() override {
        KHIIN_TRACE("");
        EndUIElement();
        m_service = nullptr;
        m_context = nullptr;
        DestroyCandidateWindow();
        m_candidate_window.reset(nullptr);
        m_pager.reset(nullptr);
    }

    virtual void DestroyCandidateWindow() override {
        KHIIN_TRACE("");
        if (m_candidate_window && m_candidate_window->hwnd()) {
            ::DestroyWindow(m_candidate_window->hwnd());
        }
    }

    virtual void Update(ITfContext *pContext, EditState edit_state, const CandidateList &candidate_list,
                        RECT text_rect) override {
        KHIIN_TRACE("");
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
        KHIIN_TRACE("");
        return m_candidate_window && m_candidate_window->Showing();
    }

    virtual bool Selecting() override {
        KHIIN_TRACE("");
        return m_edit_state == EditState::EDIT_SELECTING;
    }

    virtual bool MultiColumn() override {
        KHIIN_TRACE("");
        return m_candidate_grid.size() > 1;
    }

    virtual int PageCount() override {
        KHIIN_TRACE("");
        return m_pager->PageCount();
    }

    virtual int MaxQuickSelect() override {
        KHIIN_TRACE("");
        auto focused_col = m_pager->GetFocusedColumnIndex();
        if (focused_col < m_candidate_grid.size()) {
            return static_cast<int>(m_candidate_grid.at(focused_col).size());
        }
        return 0;
    }

    virtual int QuickSelect(int index) override {
        KHIIN_TRACE("");
        auto focused_col = m_pager->GetFocusedColumnIndex();

        if (focused_col < m_candidate_grid.size()) {
            auto &col = m_candidate_grid.at(focused_col);
            if (index < static_cast<int>(col.size())) {
                auto cand = col.at(index);
                return cand->id();
            }
        }

        return -1;
    }

    virtual int RotateNext() override {
        KHIIN_TRACE("");
        if (m_pager->PageCount() < 2) {
            return -1;
        }

        return m_pager->NextPageCandidateId();
    }

    virtual int RotatePrev() override {
        KHIIN_TRACE("");
        if (m_pager->PageCount() < 2) {
            return -1;
        }

        return m_pager->PrevPageCandidateId();
    }

    virtual void Show() override {
        KHIIN_TRACE("");
        if (m_candidate_window && m_showable) {
            m_candidate_window->Show();
        }
    }

    virtual void Hide() override {
        KHIIN_TRACE("");
        if (m_candidate_window) {
            m_candidate_window->Hide();
        }
    }

    virtual void OnSetThreadFocus() override {
        KHIIN_TRACE("");
        if (Showing() && m_showable) {
            Show();
        }
    }

    virtual void OnKillThreadFocus() override {
        KHIIN_TRACE("");
        Hide();
    }

    virtual ITfContext *context() {
        KHIIN_TRACE("");
        return m_context.get();
    }

    //+---------------------------------------------------------------------------
    //
    // CandidateSelectListener
    //
    //----------------------------------------------------------------------------

    virtual void OnSelectCandidate(int32_t id) override {
        KHIIN_TRACE("");
        m_service->OnCandidateSelected(id);
    }

    //+---------------------------------------------------------------------------
    //
    // ConfigChangeListener
    //
    //----------------------------------------------------------------------------

    virtual void OnConfigChanged(AppConfig *config) override {
        KHIIN_TRACE("");
        if (m_candidate_window) {
            m_candidate_window->OnConfigChanged(config);
        }
    }

    //+---------------------------------------------------------------------------
    //
    // ITfUIElement
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP GetDescription(BSTR *pbstrDescription) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        BSTR bstr = ::SysAllocString(kCandidateWindowClassName.data());
        pbstrDescription = &bstr;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetGUID(GUID *pguid) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        *pguid = kCandidateWindowGuid;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP Show(BOOL bShow) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
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
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
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

    virtual STDMETHODIMP GetUpdatedFlags(DWORD *flags) override {
        KHIIN_TRACE("");
        *flags = 0;

        if (!CurrAndPrevCandidatesEq()) {
            *flags = (TF_CLUIE_STRING | TF_CLUIE_COUNT);
        }

        *flags = (TF_CLUIE_CURRENTPAGE | TF_CLUIE_PAGEINDEX | TF_CLUIE_SELECTION);

        return S_OK;
    }

    virtual STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppdim) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (ppdim == nullptr) {
            return E_INVALIDARG;
        }
        check_hresult(m_context->GetDocumentMgr(ppdim));
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetCount(UINT *puCount) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (puCount == nullptr) {
            return E_INVALIDARG;
        }
        *puCount = m_candidate_list.candidates_size();
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetSelection(UINT *puIndex) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (puIndex == nullptr) {
            return E_INVALIDARG;
        }
        *puIndex = m_candidate_list.focused();
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetString(UINT uIndex, BSTR *pstr) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (pstr == nullptr) {
            return E_INVALIDARG;
        }
        if (uIndex >= static_cast<uint32_t>(m_candidate_list.candidates_size())) {
            KHIIN_ERROR("index too big for candidate list");
            return E_FAIL;
        }
        auto cand_str = m_candidate_list.candidates().at(uIndex).value();
        auto cand_wstr = Utils::Widen(cand_str);
        BSTR cand_bstr = ::SysAllocStringLen(cand_wstr.data(), static_cast<uint32_t>(cand_wstr.size()));
        *pstr = cand_bstr;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
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

        for (uint32_t i = 0; i < *puPageCnt; ++i) {
            pIndex[i] = i * m_pager->MaxPageSize();
        }

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP SetPageIndex(UINT *pIndex, UINT uPageCnt) override {
        KHIIN_TRACE("");
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetCurrentPage(UINT *puPage) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (puPage == nullptr) {
            return E_INVALIDARG;
        }
        *puPage = m_pager->CurrentPageIndex();
        CATCH_FOR_HRESULT;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCandidateListUIElementBehavior
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP SetSelection(UINT nIndex) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (nIndex >= static_cast<uint32_t>(m_candidate_list.candidates_size())) {
            return E_INVALIDARG;
        }

        OnSelectCandidate(nIndex);
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP Finalize(void) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        auto cmd = Command::default_instance().New();
        cmd->mutable_request()->set_type(CommandType::COMMIT);
        EditSession::HandleAction(m_service.get(), m_context.get(), cmd);
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP Abort(void) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        auto cmd = Command::default_instance().New();
        cmd->mutable_request()->set_type(CommandType::RESET);
        EditSession::HandleAction(m_service.get(), m_context.get(), cmd);
        return S_OK;
        CATCH_FOR_HRESULT;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfIntegratableCandidateListUIElement
    //
    //----------------------------------------------------------------------------
    // virtual STDMETHODIMP SetIntegrationStyle(GUID guidIntegrationStyle) override {
    //    return E_NOTIMPL;
    //}
    // virtual STDMETHODIMP GetSelectionStyle(TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle) override {
    //    return E_NOTIMPL;
    //}
    // virtual STDMETHODIMP OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
    //    return E_NOTIMPL;
    //}
    // virtual STDMETHODIMP ShowCandidateNumbers(BOOL *pfShow) override {
    //    return E_NOTIMPL;
    //}
    // virtual STDMETHODIMP FinalizeExactCompositionString(void) override {
    //    return E_NOTIMPL;
    //}

  private:
    void RegisterCandidateWindow() {
        KHIIN_TRACE("");
        auto contextView = com_ptr<ITfContextView>();
        check_hresult(m_context->GetActiveView(contextView.put()));

        HWND parentWnd;
        check_hresult(contextView->GetWnd(&parentWnd));

        if (!(parentWnd)) {
            parentWnd = ::GetFocus();
        }

        m_candidate_window = std::unique_ptr<CandidateWindow>(CandidateWindow::Create(parentWnd));
        OnConfigChanged(m_service->config());
        m_candidate_window->RegisterCandidateSelectListener(this);
    }

    com_ptr<ITfUIElementMgr> GetUiElementMgr() {
        KHIIN_TRACE("");
        auto threadmgr = m_service->thread_mgr();
        if (!threadmgr) {
            return nullptr;
        }
        auto uielement_mgr = com_ptr<ITfUIElementMgr>();
        check_hresult(threadmgr->QueryInterface(IID_ITfUIElementMgr, uielement_mgr.put_void()));
        return uielement_mgr;
    }

    void BeginUIElement() {
        if (auto uielement_mgr = GetUiElementMgr()) {
            KHIIN_TRACE("");
            BOOL showable = TRUE;
            check_hresult(uielement_mgr->BeginUIElement(this, &showable, &m_element_id));
            m_showable = showable;
            KHIIN_DEBUG("m_showable: {}", m_showable);
        }
    }

    void UpdateUIElement() {
        if (auto uielement_mgr = GetUiElementMgr()) {
            KHIIN_TRACE("");
            check_hresult(uielement_mgr->UpdateUIElement(m_element_id));
        }
    }

    void EndUIElement() {
        if (auto uielement_mgr = GetUiElementMgr()) {
            KHIIN_TRACE("");
            check_hresult(uielement_mgr->EndUIElement(m_element_id));
        }
    }

    void UpdateWindow() {
        KHIIN_TRACE("");
        m_candidate_grid.clear();
        m_pager->GetPage(m_candidate_grid);
        bool qs_active = m_edit_state == EditState::EDIT_SELECTING;
        auto focused_cand_id = m_pager->GetFocusedCandidateId();
        auto focused_col = m_pager->GetFocusedColumnIndex();
        auto display_mode = m_pager->GetDisplayMode();

        KHIIN_DEBUG("m_text_rect: {} {}", m_text_rect.left, m_text_rect.top);

        m_candidate_window->SetCandidates(display_mode, &m_candidate_grid, focused_cand_id, focused_col, qs_active,
                                          m_text_rect);
        UpdateUIElement();
        Show();
    }

    bool DigitKeyIsQuickSelect(char key_char) {
        KHIIN_TRACE("");
        auto key_int = key_char - '0';
        auto available_qs = m_candidate_grid.at(m_focused_col).size();
        if (key_int > 0 && key_int <= static_cast<int>(available_qs)) {
            return true;
        }
        return false;
    }

    bool CurrAndPrevCandidatesEq() {
        auto &lhs = m_candidate_list;
        auto &rhs = m_prev_candidate_list;
        auto ret = true;

        if (lhs.candidates_size() != rhs.candidates_size()) {
            ret = false;
        } else if (auto size = lhs.candidates_size(); size > 0) {
            for (auto i = 0; i < size; ++i) {
                auto &lhc = lhs.candidates().at(i);
                auto &rhc = rhs.candidates().at(i);
                if (lhc.value() != rhc.value()) {
                    ret = false;
                }
            }
        }

        m_prev_candidate_list.CopyFrom(m_candidate_list);
        return ret;
    }

    com_ptr<TextService> m_service = nullptr;
    com_ptr<ITfContext> m_context = nullptr;
    CandidateList m_candidate_list = {};
    CandidateList m_prev_candidate_list = {};
    std::unique_ptr<CandidateWindow> m_candidate_window = nullptr;
    std::unique_ptr<CandidatePager> m_pager = nullptr;
    EditState m_edit_state = EditState::EDIT_EMPTY;
    CandidateGrid m_candidate_grid = CandidateGrid();
    size_t m_focused_col = 0;
    RECT m_text_rect = {};
    bool m_showable = true;
    DWORD m_element_id = (DWORD)-1;
};

} // namespace

HRESULT CandidateListUIFactory::Create(CandidateListUI **ppCandidateListUI) {
    as_self<CandidateListUI>(make_self<CandidateListUIImpl>()).copy_to(ppCandidateListUI);
    return S_OK;
}

} // namespace khiin::win32
