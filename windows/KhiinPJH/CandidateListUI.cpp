#include "pch.h"

#include "CandidateListUI.h"
#include "EditSession.h"
#include "common.h"
#include "Utils.h"

namespace khiin::win32 {

void CandidateListUI::Initialize(TextService *pTextService) {
    service.copy_from(pTextService);
}

void CandidateListUI::Uninitialize() {
    D(__FUNCTIONW__);
    service = nullptr;
    context = nullptr;
    DestroyCandidateWindow();
    candidateWindow.reset(nullptr);
}

void CandidateListUI::DestroyCandidateWindow() {
    D(__FUNCTIONW__);
    if (candidateWindow && candidateWindow->hwnd()) {
        ::DestroyWindow(candidateWindow->hwnd());
    }
}

void CandidateListUI::Update(ITfContext *pContext, const messages::CandidateList &candidate_list, RECT text_rect) {
    D(__FUNCTIONW__);
    context.copy_from(pContext);
    m_candidate_list.CopyFrom(candidate_list);

    if (!candidateWindow) {
        makeCandidateWindow();
    }

    candidateWindow->SetScreenCoordinates(text_rect);
    candidateWindow->SetCandidates(&m_candidate_list);
    candidateWindow->Show();
}

//+---------------------------------------------------------------------------
//
// ITfUIElement
//
//----------------------------------------------------------------------------

STDMETHODIMP CandidateListUI::GetDescription(BSTR *pbstrDescription) {
    TRY_FOR_HRESULT;
    BSTR bstr = ::SysAllocString(kCandidateWindowClassName.data());
    pbstrDescription = &bstr;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP CandidateListUI::GetGUID(GUID *pguid) {
    TRY_FOR_HRESULT;
    *pguid = kCandidateWindowGuid;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP CandidateListUI::Show(BOOL bShow) {
    TRY_FOR_HRESULT;
    if (!candidateWindow) {
        return S_OK;
    }

    if (bShow) {
        candidateWindow->Show();
    } else {
        candidateWindow->Hide();
    }

    CATCH_FOR_HRESULT;
}

STDMETHODIMP CandidateListUI::IsShown(BOOL *pbShow) {
    D(__FUNCTIONW__);
    TRY_FOR_HRESULT;
    if (!candidateWindow) {
        *pbShow = FALSE;
        return S_OK;
    }

    *pbShow = candidateWindow->showing();
    CATCH_FOR_HRESULT;
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

void CandidateListUI::makeCandidateWindow() {
    auto contextView = winrt::com_ptr<ITfContextView>();
    winrt::check_hresult(context->GetActiveView(contextView.put()));

    HWND parentWnd;
    winrt::check_hresult(contextView->GetWnd(&parentWnd));

    if (!(parentWnd)) {
        parentWnd = ::GetFocus();
    }

    candidateWindow = std::unique_ptr<CandidateWindow>(CandidateWindow::Create(parentWnd));
}

} // namespace khiin::win32
