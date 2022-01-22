#include "pch.h"

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "EditSession.h"
#include "TextEngine.h"
#include "TextService.h"
#include "common.h"

namespace khiin::win32 {

constexpr auto kAsyncRWFlag = TF_ES_ASYNCDONTCARE | TF_ES_READWRITE;

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
        TRY_FOR_HRESULT;
        auto compositionMgr = cast_as<CompositionMgr>(service->compositionMgr());
        auto candidateUI = cast_as<CandidateListUI>(service->candidateUI());
        bool composing = compositionMgr->composing();
        BOOL shown;
        candidateUI->IsShown(&shown);

        if (action.compMsg == Message::StartComposition && !composing) {
            compositionMgr->StartComposition(ec, context.get());
        }

        if (action.compMsg == Message::UpdateComposition || action.compMsg == Message::StartComposition) {
            compositionMgr->DoComposition(ec, context.get(), action.text);
        }

        if (action.compMsg == Message::CommitText) {
            compositionMgr->EndComposition(ec);
        }

        if (action.candMsg == Message::ShowCandidates) {
            auto range = winrt::com_ptr<ITfRange>();
            compositionMgr->GetTextRange(ec, range.put());

            auto contextView = winrt::com_ptr<ITfContextView>();
            winrt::check_hresult(context->GetActiveView(contextView.put()));

            RECT rect;
            BOOL clipped;
            winrt::check_hresult(contextView->GetTextExt(ec, range.get(), &rect, &clipped));

            candidateUI->Update(context.get(), action.candidates, std::move(rect));

            if (!shown) {
                winrt::check_hresult(candidateUI->Show(true));
                
            }
        } else if (action.candMsg == Message::HideCandidates) {
            if (shown) {
                winrt::check_hresult(candidateUI->Show(false));
            }
        }

        return S_OK;
        CATCH_FOR_HRESULT;
    }

  private:
    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    Action action;
};

void EditSession::HandleAction(TextService *pService, ITfContext *pContext, Action action) {
    D(__FUNCTIONW__);
    auto session = winrt::make_self<EditSessionImpl>(pService, pContext, action);

    auto sessionHr = E_FAIL;
    winrt::check_hresult(
        pContext->RequestEditSession(pService->clientId(), session.get(), kAsyncRWFlag, &sessionHr));
    winrt::check_hresult(sessionHr);
}

} // namespace khiin::win32
