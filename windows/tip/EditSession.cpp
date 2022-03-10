#include "pch.h"

#include "proto/proto.h"

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "EditSession.h"
#include "EngineController.h"
#include "TextService.h"

namespace khiin::win32::tip {
namespace {
using namespace khiin::proto;

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

constexpr auto kAsyncRWFlag = TF_ES_ASYNCDONTCARE | TF_ES_READWRITE;

struct EditSessionImpl : winrt::implements<EditSessionImpl, ITfEditSession> {
    EditSessionImpl(TextService *pService, ITfContext *pContext, Command *command) : command(command) {
        KHIIN_TRACE("");
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
        KHIIN_TRACE("");
        TRY_FOR_HRESULT;
        auto composition_mgr = service->composition_mgr();
        auto candidate_ui = service->candidate_ui();
        bool composing = composition_mgr->composing();
        bool Showing = candidate_ui->Showing();
        auto cmd_type = command->request().type();
        auto &response = command->response();

        if (response.error() == ErrorCode::FAIL) {
            composition_mgr->CommitComposition(ec, context.get());
        } else if (cmd_type == CMD_COMMIT) {
            if (response.preedit().segments().size() == 0) {
                composition_mgr->CommitComposition(ec, context.get());
            } else {
                composition_mgr->CommitComposition(ec, context.get(), response.preedit());
            }
        } else if (cmd_type == CMD_SEND_KEY || cmd_type == CMD_SELECT_CANDIDATE ||
                   cmd_type == CMD_FOCUS_CANDIDATE) {
            composition_mgr->DoComposition(ec, context.get(), response.preedit());

            if (response.candidate_list().candidates().size() > 0) {
                candidate_ui->Update(context.get(), response.edit_state(), response.candidate_list(),
                                     GetEditRect(ec, composition_mgr.get(), context.get()));
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
    Command *command;
};

void EditSession::HandleAction(TextService *pService, ITfContext *pContext, Command *command) {
    KHIIN_TRACE("");
    auto session = winrt::make_self<EditSessionImpl>(pService, pContext, command);
    auto sessionHr = E_FAIL;
    winrt::check_hresult(pContext->RequestEditSession(pService->clientId(), session.get(), kAsyncRWFlag, &sessionHr));
    winrt::check_hresult(sessionHr);
}

} // namespace khiin::win32
