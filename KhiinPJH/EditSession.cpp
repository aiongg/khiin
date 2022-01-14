#include "pch.h"

#include "EditSession.h"

namespace Khiin {

HRESULT EditSession::request(TfClientId clientId, ITfContext *pContext, CallbackFn callback) {
    auto hr = E_FAIL;
    auto session = winrt::make_self<EditSession>();
    hr = session->init(callback);
    CHECK_RETURN_HRESULT(hr);

    auto hrSession = E_FAIL;
    hr = pContext->RequestEditSession(clientId, session.get(), TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
    CHECK_RETURN_HRESULT(hrSession);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT EditSession::init(CallbackFn callback) {
    D(__FUNCTIONW__);
    this->callback = callback;
    return S_OK;
}

HRESULT EditSession::uninit() {
    D(__FUNCTIONW__);
    callback = nullptr;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfEditSession
//
//----------------------------------------------------------------------------

STDMETHODIMP EditSession::DoEditSession(TfEditCookie cookie) {
    D(__FUNCTIONW__);
    auto hr = callback(cookie);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

} // namespace Khiin
