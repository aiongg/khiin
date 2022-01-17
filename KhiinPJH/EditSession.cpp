#include "pch.h"

#include "EditSession.h"
#include "ITextService.h"
#include "CompositionMgr.h"
#include "common.h"
#include "ITextEngine.h"
#include "CandidateListUI.h"

namespace Khiin {

constexpr auto AsyncDontCareRW = TF_ES_ASYNCDONTCARE | TF_ES_READWRITE;

struct EditSessionImpl : winrt::implements<EditSessionImpl, ITfEditSession> {
    EditSessionImpl(ITextService *pService, ITfContext *pContext, KeyEvent keyEvent) : keyEvent(keyEvent) {
        service.copy_from(pService);
        context.copy_from(pContext);
    }
    ~EditSessionImpl() = default;

    // ITfEditSession
    virtual STDMETHODIMP DoEditSession(TfEditCookie ec) override {
        auto hr = E_FAIL;

        auto compositionMgr = cast_as<CompositionMgr>(service->compositionMgr());
        hr = compositionMgr->doComposition(ec, context.get(), service->engine()->buffer());
        CHECK_RETURN_HRESULT(hr);

        auto candidateUI = cast_as<CandidateListUI>(service->candidateUI());

        return S_OK;
    }

  private:
    winrt::com_ptr<ITextService> service = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    KeyEvent keyEvent;
};

HRESULT EditSession2::request(ITextService *pService, ITfContext *pContext, KeyEvent keyEvent) {
    auto session = winrt::make_self<EditSessionImpl>();
    auto sessionHr = E_FAIL;
    auto hr = pContext->RequestEditSession(pService->clientId(), session.get(), AsyncDontCareRW, &sessionHr);
    CHECK_RETURN_HRESULT(hr);
    CHECK_RETURN_HRESULT(sessionHr);

    return S_OK;
}

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
