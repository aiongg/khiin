#include "pch.h"

#include "CandidateListUI.h"

#include "proto/proto.h"

#include "CandidatePager.h"
#include "CandidateWindow.h"
#include "Colors.h"
#include "CompositionMgr.h"
#include "Config.h"
#include "EditSession.h"
#include "Guids.h"
#include "TextService.h"
#include "Utils.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;
using namespace khiin::proto;

bool IsArrowKey(int key_code) {
    return key_code == VK_LEFT || key_code == VK_UP || key_code == VK_RIGHT || key_code == VK_DOWN;
}

struct CandidateListUIImpl :
    public implements<CandidateListUIImpl, ITfCandidateListUIElementBehavior, CandidateListUI>,
    CandidateSelectListener,
    ConfigChangeListener {
    void Initialize(TextService *service) override {
        KHIIN_TRACE("");
        m_service.copy_from(service);
        m_service->RegisterConfigChangeListener(this);
        m_pager = std::unique_ptr<CandidatePager>(CandidatePager::Create());
        BeginUIElement();
    }

    void Uninitialize() override {
        KHIIN_TRACE("");
        EndUIElement();
        m_service = nullptr;
        m_context = nullptr;
        DestroyCandidateWindow();
        m_candidate_window.reset(nullptr);
        m_pager.reset(nullptr);
    }

    void DestroyCandidateWindow() override {
        KHIIN_TRACE("");
        if (m_candidate_window && m_candidate_window->hwnd()) {
            ::DestroyWindow(m_candidate_window->hwnd());
        }
    }

    void Update(ITfContext *context, EditState edit_state, const CandidateList &candidate_list,
                RECT text_rect) override {
        KHIIN_TRACE("");
        m_context.copy_from(context);
        m_candidate_list.CopyFrom(candidate_list);
        m_edit_state = edit_state;
        m_text_rect = text_rect;

        if (!m_candidate_window) {
            RegisterCandidateWindow();
        }

        if (m_edit_state == ES_COMPOSING) {
            m_pager->SetCandidateList(&m_candidate_list);
        } else if (m_edit_state == ES_SELECTING) {
            m_pager->SetFocus(m_candidate_list.focused());
        }

        UpdateWindow();
    }

    void Move(TfEditCookie cookie) override {
        auto view = winrt::com_ptr<ITfContextView>();
        check_hresult(m_context->GetActiveView(view.put()));
        auto range = m_service->composition_mgr()->GetTextRange(cookie);

        if (!range) {
            return;
        }

        check_hresult(range->Collapse(cookie, TF_ANCHOR_START));
        LONG shifted;
        check_hresult(range->ShiftStart(cookie, 0, &shifted, nullptr));
        check_hresult(range->ShiftEnd(cookie, 1, &shifted, nullptr));
        auto rect = RECT();
        BOOL clipped = FALSE;
        check_hresult(view->GetTextExt(cookie, range.get(), &rect, &clipped));
        m_candidate_window->Move(rect);
        KHIIN_DEBUG("after update: {} {}", rect.left, rect.top);
    }

    bool Showing() override {
        KHIIN_TRACE("");
        return m_candidate_window && m_candidate_window->Showing();
    }

    bool Selecting() override {
        KHIIN_TRACE("");
        return m_edit_state == ES_SELECTING;
    }

    bool MultiColumn() override {
        KHIIN_TRACE("");
        return m_candidate_grid.size() > 1;
    }

    int PageCount() override {
        KHIIN_TRACE("");
        return m_pager->PageCount();
    }

    int MaxQuickSelect() override {
        KHIIN_TRACE("");
        auto focused_col = m_pager->GetFocusedColumnIndex();
        if (focused_col < m_candidate_grid.size()) {
            return static_cast<int>(m_candidate_grid.at(focused_col).size());
        }
        return 0;
    }

    int QuickSelect(int index) override {
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

    int RotateNext() override {
        KHIIN_TRACE("");
        if (m_pager->PageCount() < 2) {
            return -1;
        }

        return m_pager->NextPageCandidateId();
    }

    int RotatePrev() override {
        KHIIN_TRACE("");
        if (m_pager->PageCount() < 2) {
            return -1;
        }

        return m_pager->PrevPageCandidateId();
    }

    void Show() override {
        KHIIN_TRACE("");
        if (m_candidate_window && m_showable) {
            m_candidate_window->Show();
        }
    }

    void Hide() override {
        KHIIN_TRACE("");
        if (m_candidate_window) {
            m_candidate_window->Hide();
        }
    }

    void OnSetThreadFocus() override {
        KHIIN_TRACE("");
        if (Showing() && m_showable) {
            Show();
        }
    }

    void OnKillThreadFocus() override {
        KHIIN_TRACE("");
        Hide();
    }

    ITfContext *context() {
        KHIIN_TRACE("");
        return m_context.get();
    }

    //+---------------------------------------------------------------------------
    //
    // CandidateSelectListener
    //
    //----------------------------------------------------------------------------

    void OnSelectCandidate(int32_t id) override {
        KHIIN_TRACE("");
        m_service->OnCandidateSelected(id);
    }

    //+---------------------------------------------------------------------------
    //
    // ConfigChangeListener
    //
    //----------------------------------------------------------------------------

    void OnConfigChanged(AppConfig *config) override {
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

    STDMETHODIMP GetDescription(BSTR *pbstrDescription) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        BSTR bstr = ::SysAllocString(kCandidateWindowClassName.data());
        pbstrDescription = &bstr;
        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP GetGUID(GUID *pguid) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        *pguid = guids::kCandidateWindow;
        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP Show(BOOL bShow) override {
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

    STDMETHODIMP IsShown(BOOL *pbShow) override {
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

    STDMETHODIMP GetUpdatedFlags(DWORD *flags) override {
        KHIIN_TRACE("");
        *flags = 0;

        if (!CurrAndPrevCandidatesEq()) {
            *flags = (TF_CLUIE_STRING | TF_CLUIE_COUNT);
        }

        *flags = (TF_CLUIE_CURRENTPAGE | TF_CLUIE_PAGEINDEX | TF_CLUIE_SELECTION);

        return S_OK;
    }

    STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppdim) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (ppdim == nullptr) {
            return E_INVALIDARG;
        }
        check_hresult(m_context->GetDocumentMgr(ppdim));
        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP GetCount(UINT *puCount) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (puCount == nullptr) {
            return E_INVALIDARG;
        }
        *puCount = m_candidate_list.candidates_size();
        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP GetSelection(UINT *puIndex) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (puIndex == nullptr) {
            return E_INVALIDARG;
        }
        *puIndex = m_candidate_list.focused();
        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP GetString(UINT uIndex, BSTR *pstr) override {
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

    STDMETHODIMP GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt) override {
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

    STDMETHODIMP SetPageIndex(UINT *pIndex, UINT uPageCnt) override {
        KHIIN_TRACE("");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetCurrentPage(UINT *puPage) override {
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

    STDMETHODIMP SetSelection(UINT nIndex) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        if (nIndex >= static_cast<uint32_t>(m_candidate_list.candidates_size())) {
            return E_INVALIDARG;
        }

        OnSelectCandidate(nIndex);
        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP Finalize(void) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        m_service->CommitComposition();
        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP Abort(void) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");
        m_service->Reset();
        return S_OK;
        CATCH_FOR_HRESULT;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfIntegratableCandidateListUIElement
    //
    //----------------------------------------------------------------------------
    // STDMETHODIMP SetIntegrationStyle(GUID guidIntegrationStyle) override {
    //    return E_NOTIMPL;
    //}
    // STDMETHODIMP GetSelectionStyle(TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle) override {
    //    return E_NOTIMPL;
    //}
    // STDMETHODIMP OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
    //    return E_NOTIMPL;
    //}
    // STDMETHODIMP ShowCandidateNumbers(BOOL *pfShow) override {
    //    return E_NOTIMPL;
    //}
    // STDMETHODIMP FinalizeExactCompositionString(void) override {
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
        bool qs_active = m_edit_state == ES_SELECTING;
        auto focused_cand_id = m_pager->GetFocusedCandidateId();
        auto focused_col = m_pager->GetFocusedColumnIndex();
        auto display_mode = m_pager->GetDisplayMode();

        KHIIN_DEBUG("cand grid size: {}", m_candidate_grid.size());

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
    EditState m_edit_state = ES_EMPTY;
    CandidateGrid m_candidate_grid = CandidateGrid();
    size_t m_focused_col = 0;
    RECT m_text_rect = {};
    bool m_showable = true;
    DWORD m_element_id = (DWORD)-1;
};

} // namespace

com_ptr<CandidateListUI> CandidateListUI::Create() {
    return as_self<CandidateListUI>(make_self<CandidateListUIImpl>());
}

} // namespace khiin::win32::tip
