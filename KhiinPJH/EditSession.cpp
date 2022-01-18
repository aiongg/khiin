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
        auto candidateUI = cast_as<CandidateListUI>(service->candidateUI());
        bool composing = compositionMgr->composing();
        BOOL shown;
        candidateUI->IsShown(&shown);

        if (action.compMsg == Message::StartComposition && !composing) {
            hr = compositionMgr->startComposition(ec, context.get());
            CHECK_RETURN_HRESULT(hr);
        }

        if (action.compMsg == Message::UpdateComposition || action.compMsg == Message::StartComposition) {
            hr = compositionMgr->doComposition(ec, context.get(), action.text);
            CHECK_RETURN_HRESULT(hr);
        }

        if (action.compMsg == Message::CommitText) {
            hr = compositionMgr->endComposition(ec);
            CHECK_RETURN_HRESULT(hr);
        }

        if (action.candMsg == Message::ShowCandidates) {
            hr = candidateUI->update(context.get(), action.candidates);
            CHECK_RETURN_HRESULT(hr);

            if (!shown) {
                hr = candidateUI->Show(true);
                CHECK_RETURN_HRESULT(hr);
            }
        } else if (action.candMsg == Message::HideCandidates) {
            if (shown) {
                hr = candidateUI->Show(false);
                CHECK_RETURN_HRESULT(hr);
            }
        }

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
