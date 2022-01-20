#include "pch.h"

#include "CandidateListUI.h"
#include "EditSession.h"
#include "common.h"

namespace Khiin {

void CandidateListUI::Initialize(TextService *pTextService) {
    service.copy_from(pTextService);
}

void CandidateListUI::Uninitialize() {
    D(__FUNCTIONW__);
    service = nullptr;
    context = nullptr;
    candidateWindow->Destroy();
    candidateWindow.reset(nullptr);
}

void CandidateListUI::DestroyCandidateWindow() {
    D(__FUNCTIONW__);
    candidateWindow->Destroy();
}

void CandidateListUI::Update(ITfContext *pContext, std::vector<std::string> candidates, RECT text_rect) {
    D(__FUNCTIONW__);
    context.copy_from(pContext);

    if (!candidateWindow) {
        makeCandidateWindow();
    }

    candidateList.clear();
    for (auto &c : candidates) {
        auto wcand_size = ::MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, c.data(), static_cast<int>(c.size()), NULL, 0);
        auto wcand = std::wstring(wcand_size, '\0');
        ::MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, c.data(), static_cast<int>(c.size()), &wcand[0], wcand_size);
        candidateList.push_back(std::move(wcand));
    }
    candidateWindow->SetScreenCoordinates(std::move(text_rect));
    candidateWindow->SetCandidates(&candidateList);
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

    candidateWindow = std::make_unique<CandidateWindow>(parentWnd);
    candidateWindow->Create();
}

} // namespace Khiin
