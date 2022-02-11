#include "pch.h"

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "EditSession.h"
#include "EngineController.h"
#include "TextService.h"

namespace khiin::win32 {
namespace {

RECT GetEditRect(TfEditCookie ec, CompositionMgr *composition_mgr, ITfContext *context) {
    auto range = winrt::com_ptr<ITfRange>();
    composition_mgr->GetTextRange(ec, range.put());

    auto contextView = winrt::com_ptr<ITfContextView>();
    winrt::check_hresult(context->GetActiveView(contextView.put()));

    RECT rect;
    BOOL clipped;
    winrt::check_hresult(contextView->GetTextExt(ec, range.get(), &rect, &clipped));
    return rect;
}

} // namespace

using namespace khiin::messages;

constexpr auto kAsyncRWFlag = TF_ES_ASYNCDONTCARE | TF_ES_READWRITE;

struct EditSessionImpl : winrt::implements<EditSessionImpl, ITfEditSession> {
    EditSessionImpl(TextService *pService, ITfContext *pContext, Command &&command) {
        D(__FUNCTIONW__);
        service.copy_from(pService);
        context.copy_from(pContext);
        this->command = std::move(command);
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
        auto composition_mgr = cast_as<CompositionMgr>(service->composition_mgr());
        auto candidate_ui = cast_as<CandidateListUI>(service->candidate_ui());
        bool composing = composition_mgr->composing();
        bool Showing = candidate_ui->Showing();

        if (command.output().error() == ErrorCode::FAIL) {
            composition_mgr->CommitComposition(ec, context.get());
        } else if (command.type() == CommandType::COMMIT) {
            if (command.output().preedit().segments().size() == 0) {
                composition_mgr->CommitComposition(ec, context.get());
            } else {
                composition_mgr->CommitComposition(ec, context.get(), command.output().preedit());
            }
        } else if (command.type() == CommandType::SEND_KEY) {
            composition_mgr->DoComposition(ec, context.get(), command.output().preedit());

            if (command.output().candidate_list().candidates().size() > 0) {
                candidate_ui->Update(context.get(), command.output().edit_state(), command.output().candidate_list(),
                                     GetEditRect(ec, composition_mgr, context.get()));
                if (!Showing) {
                    candidate_ui->Show();
                }
            } else {
                candidate_ui->Hide();
            }
        }

        return S_OK;
        CATCH_FOR_HRESULT;
    }

  private:
    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    Command command;
};

void EditSession::HandleAction(TextService *pService, ITfContext *pContext, Command &&command) {
    D(__FUNCTIONW__);
    auto session = winrt::make_self<EditSessionImpl>(pService, pContext, std::move(command));
    auto sessionHr = E_FAIL;
    winrt::check_hresult(pContext->RequestEditSession(pService->clientId(), session.get(), kAsyncRWFlag, &sessionHr));
    winrt::check_hresult(sessionHr);
}

} // namespace khiin::win32
