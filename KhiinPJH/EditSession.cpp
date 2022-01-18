#include "pch.h"

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "EditSession.h"
#include "TextEngine.h"
#include "TextService.h"
#include "common.h"

namespace Khiin {

constexpr auto AsyncDontCareRW = TF_ES_ASYNCDONTCARE | TF_ES_READWRITE;

struct EditSessionImpl : winrt::implements<EditSessionImpl, ITfEditSession> {
    EditSessionImpl(TextService *pService, ITfContext *pContext, Action action) : action(action) {
        D(__FUNCTIONW__);
        service.copy_from(pService);
        context.copy_from(pContext);
    }
    ~EditSessionImpl() = default;

    //+---------------------------------------------------------------------------
    //
    // ITfEditSession
    //
    //----------------------------------------------------------------------------
    
    virtual STDMETHODIMP DoEditSession(TfEditCookie ec) override {
        D(__FUNCTIONW__);
        auto hr = E_FAIL;
        auto compositionMgr = cast_as<CompositionMgr>(service->compositionMgr());

        if (action.msg == Message::StartComposition) {
            hr = compositionMgr->startComposition(ec, context.get());
            CHECK_RETURN_HRESULT(hr);

            hr = compositionMgr->doComposition(ec, context.get(), action.text);
            CHECK_RETURN_HRESULT(hr);
        } else if (action.msg == Message::UpdateComposition) {
            hr = compositionMgr->doComposition(ec, context.get(), action.text);
            CHECK_RETURN_HRESULT(hr);
        } else if (action.msg == Message::CommitText) {
            hr = compositionMgr->endComposition(ec);
            CHECK_RETURN_HRESULT(hr);
        }

        auto candidateUI = cast_as<CandidateListUI>(service->candidateUI());

        return S_OK;
    }

  private:
    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    Action action;
};

HRESULT EditSession::handleAction(TextService *pService, ITfContext *pContext, Action action) {
    D(__FUNCTIONW__);
    auto session = winrt::make_self<EditSessionImpl>(pService, pContext, action);
    auto sessionHr = E_FAIL;
    auto hr = pContext->RequestEditSession(pService->clientId(), session.get(), AsyncDontCareRW, &sessionHr);
    CHECK_RETURN_HRESULT(hr);
    CHECK_RETURN_HRESULT(sessionHr);

    return S_OK;
}

} // namespace Khiin
