#include "pch.h"

#include "CandidateListUI.h"
#include "EditSession.h"
#include "common.h"

namespace Khiin {

CandidateListUI::~CandidateListUI() {
    uninit();
}

HRESULT CandidateListUI::init(TextService *pTextService) {
    service.copy_from(pTextService);
    candidateWindow = std::make_unique<CandidateWindow>();

    return S_OK;
}

HRESULT CandidateListUI::uninit() {
    service = nullptr;
    candidateWindow->destroy();
    candidateWindow.reset();
    return S_OK;
}

HRESULT CandidateListUI::update(ITfContext *pContext, std::vector<std::string> &&candidates) {
    auto hr = E_FAIL;
    auto contextView = winrt::com_ptr<ITfContextView>();
    hr = pContext->GetActiveView(contextView.put());
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

//+---------------------------------------------------------------------------
//
// ITfUIElement
//
//----------------------------------------------------------------------------

STDMETHODIMP CandidateListUI::GetDescription(BSTR *pbstrDescription) {
    BSTR bstr = ::SysAllocString(CandidateWindow::className.data());
    pbstrDescription = &bstr;
    return S_OK;
}

STDMETHODIMP CandidateListUI::GetGUID(GUID *pguid) {
    *pguid = CandidateWindow::guid;
    return S_OK;
}

STDMETHODIMP CandidateListUI::Show(BOOL bShow) {
    auto hr = E_FAIL;

    if (bShow) {
        hr = candidateWindow->show();
        CHECK_RETURN_HRESULT(hr);
    } else {
        hr = candidateWindow->hide();
        CHECK_RETURN_HRESULT(hr);
    }

    return S_OK;
}

STDMETHODIMP CandidateListUI::IsShown(BOOL *pbShow) {
    *pbShow = candidateWindow->showing();
    return S_OK;
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