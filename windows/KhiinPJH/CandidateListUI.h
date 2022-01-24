#pragma once

#include "CandidateWindow.h"
#include "TextService.h"
#include "common.h"

namespace khiin::win32 {

struct CandidateListUI :
    winrt::implements<CandidateListUI, ITfCandidateListUIElementBehavior, ITfIntegratableCandidateListUIElement> {
    CandidateListUI() = default;
    CandidateListUI(const CandidateListUI &) = delete;
    CandidateListUI &operator=(const CandidateListUI &) = delete;
    ~CandidateListUI() = default;

    void Initialize(TextService *pTextService);
    void Uninitialize();
    void DestroyCandidateWindow();
    void Update(ITfContext *pContext, const messages::CandidateList &candidate_list, RECT text_rect);

    // ITfUIElement
    virtual STDMETHODIMP GetDescription(BSTR *pbstrDescription) override;
    virtual STDMETHODIMP GetGUID(GUID *pguid) override;
    virtual STDMETHODIMP Show(BOOL bShow) override;
    virtual STDMETHODIMP IsShown(BOOL *pbShow) override;

    // ITfCandidateListUIElement
    virtual STDMETHODIMP GetUpdatedFlags(DWORD *pdwFlags) override;
    virtual STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppdim) override;
    virtual STDMETHODIMP GetCount(UINT *puCount) override;
    virtual STDMETHODIMP GetSelection(UINT *puIndex) override;
    virtual STDMETHODIMP GetString(UINT uIndex, BSTR *pstr) override;
    virtual STDMETHODIMP GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt) override;
    virtual STDMETHODIMP SetPageIndex(UINT *pIndex, UINT uPageCnt) override;
    virtual STDMETHODIMP GetCurrentPage(UINT *puPage) override;

    // ITfCandidateListUIElementBehavior
    virtual STDMETHODIMP SetSelection(UINT nIndex) override;
    virtual STDMETHODIMP Finalize(void) override;
    virtual STDMETHODIMP Abort(void) override;

    // ITfIntegratableCandidateListUIElement
    virtual STDMETHODIMP SetIntegrationStyle(GUID guidIntegrationStyle) override;
    virtual STDMETHODIMP GetSelectionStyle(TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle) override;
    virtual STDMETHODIMP OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP ShowCandidateNumbers(BOOL *pfShow) override;
    virtual STDMETHODIMP FinalizeExactCompositionString(void) override;

  private:
    void makeCandidateWindow();

    std::unique_ptr<CandidateWindow> candidateWindow = nullptr;
    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    messages::CandidateList m_candidate_list = {};
};

} // namespace khiin::win32
