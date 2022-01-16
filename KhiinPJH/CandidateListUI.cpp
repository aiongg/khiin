#include "pch.h"

#include "CandidateListUI.h"
#include "EditSession.h"
#include "common.h"

namespace Khiin {

HRESULT CandidateListUI::init(ITfThreadMgr *pThreadMgr) {
    auto hr = E_FAIL;

    auto docMgr = winrt::com_ptr<ITfDocumentMgr>();
    pThreadMgr->GetFocus(docMgr.put());

    auto context = winrt::com_ptr<ITfContext>();
    docMgr->GetTop(context.put());

    candidateWindow = std::make_unique<CandidateWindow>();

    auto contextView = winrt::com_ptr<ITfContextView>();
    hr = context->GetActiveView(contextView.put());
    CHECK_RETURN_HRESULT(hr);

    HWND parentWndHandle = nullptr;
    hr = contextView->GetWnd(&parentWndHandle);
    CHECK_RETURN_HRESULT(hr);

    if (parentWndHandle == nullptr) {
        parentWndHandle = ::GetFocus();
    }

    hr = candidateWindow->create(parentWndHandle);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT CandidateListUI::uninit() {
    candidateWindow->destroy();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfUIElement
//
//----------------------------------------------------------------------------

STDMETHODIMP CandidateListUI::GetDescription(BSTR *pbstrDescription) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::GetGUID(GUID *pguid) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::Show(BOOL bShow) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::IsShown(BOOL *pbShow) {
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement
//
//----------------------------------------------------------------------------

STDMETHODIMP CandidateListUI::GetUpdatedFlags(DWORD *pdwFlags) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::GetDocumentMgr(ITfDocumentMgr **ppdim) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::GetCount(UINT *puCount) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::GetSelection(UINT *puIndex) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::GetString(UINT uIndex, BSTR *pstr) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::SetPageIndex(UINT *pIndex, UINT uPageCnt) {
    return E_NOTIMPL;
}
STDMETHODIMP CandidateListUI::GetCurrentPage(UINT *puPage) {
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElementBehavior
//
//----------------------------------------------------------------------------

STDMETHODIMP CandidateListUI::SetSelection(UINT nIndex) {
    return E_NOTIMPL;
}

STDMETHODIMP CandidateListUI::Finalize(void) {
    return E_NOTIMPL;
}

STDMETHODIMP CandidateListUI::Abort(void) {
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement
//
//----------------------------------------------------------------------------

STDMETHODIMP CandidateListUI::SetIntegrationStyle(GUID guidIntegrationStyle) {
    return E_NOTIMPL;
}

STDMETHODIMP CandidateListUI::GetSelectionStyle(TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle) {
    return E_NOTIMPL;
}

STDMETHODIMP CandidateListUI::OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    return E_NOTIMPL;
}

STDMETHODIMP CandidateListUI::ShowCandidateNumbers(BOOL *pfShow) {
    return E_NOTIMPL;
}

STDMETHODIMP CandidateListUI::FinalizeExactCompositionString(void) {
    return E_NOTIMPL;
}

} // namespace Khiin