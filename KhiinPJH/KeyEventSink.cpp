#include "pch.h"

#include "KeyEventSink.h"

#include "EditSession.h"
#include "common.h"

namespace Khiin {

HRESULT KeyEventSink::init(_In_ TfClientId clientId, _In_ ITfThreadMgr *pThreadMgr,
                           _In_ CompositionMgr *pCompositionSink) {
    WINRT_ASSERT(pThreadMgr != nullptr);
    WINRT_ASSERT(pCompositionSink != nullptr);

    this->clientId = clientId;
    this->threadMgr.copy_from(pThreadMgr);
    this->compositionMgr.copy_from(pCompositionSink);
    this->keystrokeMgr = threadMgr.as<ITfKeystrokeMgr>();

    auto hr = keystrokeMgr->AdviseKeyEventSink(clientId, this, TRUE);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT KeyEventSink::uninit() {
    WINRT_ASSERT(keystrokeMgr != nullptr);

    auto hr = keystrokeMgr->UnadviseKeyEventSink(clientId);
    CHECK_RETURN_HRESULT(hr);

    clientId = TF_CLIENTID_NULL;
    threadMgr = nullptr;
    keystrokeMgr = nullptr;
    compositionMgr = nullptr;

    return S_OK;
}

HRESULT KeyEventSink::beginEditSession(ITfContext *pContext, WPARAM wParam, LPARAM lParam) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(clientId != TF_CLIENTID_NULL);
    WINRT_ASSERT(pContext != nullptr);
    WINRT_ASSERT(compositionMgr != nullptr);

    auto hr = EditSession::request(clientId, pContext, [&](TfEditCookie cookie) {
        D(__FUNCTIONW__);
        compositionMgr->setText(cookie, pContext, "Test");
        return S_OK;
    });
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink
//
//----------------------------------------------------------------------------

STDMETHODIMP KeyEventSink::OnSetFocus(BOOL fForeground) {
    D(__FUNCTIONW__);
    if (!fForeground) {
        return S_OK;
    }

    auto hr = E_FAIL;

    auto docMgr = winrt::com_ptr<ITfDocumentMgr>();
    hr = threadMgr->GetFocus(docMgr.put());
    CHECK_RETURN_HRESULT(hr);

    auto ctx = winrt::com_ptr<ITfContext>();
    hr = docMgr->GetTop(ctx.put());
    CHECK_RETURN_HRESULT(hr);

    return E_NOTIMPL;
}

STDMETHODIMP KeyEventSink::OnTestKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    D(__FUNCTIONW__);
    // if (!isKeyboardEnabled() || !isKeyboardOpen()) {
    //    D(L"Keyboard enabled: ", isKeyboardEnabled(), L"Keyboard open: ",
    //    isKeyboardOpen()); *pfEaten = FALSE; return S_OK;
    //}

    if (wParam == 0x41) {
        *pfEaten = TRUE;
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}
STDMETHODIMP KeyEventSink::OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    return E_NOTIMPL;
}
STDMETHODIMP KeyEventSink::OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    D(__FUNCTIONW__, " (W=", wParam, ")");
    if (wParam == 0x41) {
        beginEditSession(pic, wParam, lParam);
    }

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    return E_NOTIMPL;
}

STDMETHODIMP KeyEventSink::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten) {
    return E_NOTIMPL;
}

} // namespace Khiin