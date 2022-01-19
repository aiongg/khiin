#pragma once

#include "CandidateWindow.h"
#include "TextService.h"

namespace Khiin {

struct CandidateListUI :
    winrt::implements<CandidateListUI, ITfCandidateListUIElementBehavior, ITfIntegratableCandidateListUIElement> {
    CandidateListUI() = default;
    ~CandidateListUI() = default;
    DELETE_COPY_AND_ASSIGN(CandidateListUI);

    HRESULT init(TextService *pTextService);
    HRESULT uninit();
    HRESULT onCompositionTerminated();
    HRESULT update(ITfContext *pContext, std::vector<std::string> candidates);

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
    HRESULT makeCandidateWindow();

    std::unique_ptr<CandidateWindow> candidateWindow;
    winrt::com_ptr<TextService> service;
    winrt::com_ptr<ITfContext> context;
    std::vector<std::wstring> candidateList = {};
};

} // namespace Khiin
