#include "pch.h"

#include "CandidateListUI.h"
#include "EditSession.h"
#include "common.h"

namespace Khiin {

HRESULT CandidateListUI::init(TextService *pTextService) {
    service.copy_from(pTextService);
    // service->topContext(context.put());

    return S_OK;
}

HRESULT CandidateListUI::uninit() {
    D(__FUNCTIONW__);
    service = nullptr;
    context = nullptr;
    candidateWindow->Destroy();
    candidateWindow.reset(nullptr);
    return S_OK;
}

HRESULT CandidateListUI::onCompositionTerminated() {
    D(__FUNCTIONW__);
    candidateWindow->Destroy();
    return S_OK;
}

HRESULT CandidateListUI::update(ITfContext *pContext, std::vector<std::string> candidates) {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    context.copy_from(pContext);

    if (!candidateWindow) {
        hr = makeCandidateWindow();
        CHECK_RETURN_HRESULT(hr);
    }

    candidateList.clear();
    for (auto &c : candidates) {
        auto wcand_size = ::MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, c.data(), c.size(), NULL, 0);
        auto wcand = std::wstring(wcand_size, '\0');
        ::MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, c.data(), c.size(), &wcand[0], wcand_size);
        candidateList.push_back(std::move(wcand));
    }
    candidateWindow->SetCandidates(&candidateList);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfUIElement
//
//----------------------------------------------------------------------------

STDMETHODIMP CandidateListUI::GetDescription(BSTR *pbstrDescription) {
    BSTR bstr = ::SysAllocString(kCandidateWindowClassName.data());
    pbstrDescription = &bstr;
    return S_OK;
}

STDMETHODIMP CandidateListUI::GetGUID(GUID *pguid) {
    *pguid = kCandidateWindowGuid;
    return S_OK;
}

STDMETHODIMP CandidateListUI::Show(BOOL bShow) {
    auto hr = E_FAIL;

    if (!candidateWindow) {
        return S_OK;
    }

    if (bShow) {
        hr = candidateWindow->Show();
        CHECK_RETURN_HRESULT(hr);
    } else {
        hr = candidateWindow->Hide();
        CHECK_RETURN_HRESULT(hr);
    }

    return S_OK;
}

STDMETHODIMP CandidateListUI::IsShown(BOOL *pbShow) {
    D(__FUNCTIONW__);
    if (!candidateWindow) {
        *pbShow = FALSE;
        return S_OK;
    }

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

HRESULT CandidateListUI::makeCandidateWindow() {
    auto hr = E_FAIL;

    auto contextView = winrt::com_ptr<ITfContextView>();
    hr = context->GetActiveView(contextView.put());
    CHECK_RETURN_HRESULT(hr);

    HWND parentWnd;
    /*auto parentWnd = window_handle();*/
    hr = contextView->GetWnd(&parentWnd);
    CHECK_RETURN_HRESULT(hr);

    if (!(parentWnd)) {
        parentWnd = ::GetFocus();
    }

    candidateWindow = std::make_unique<CandidateWindow>(parentWnd);
    candidateWindow->Create();

    return S_OK;
}

} // namespace Khiin