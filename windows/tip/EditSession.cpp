#include "pch.h"

#include "proto/proto.h"

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "CompositionUtil.h"
#include "EditSession.h"
#include "EngineController.h"
#include "Guids.h"
#include "KeyEvent.h"
#include "TextService.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;
using namespace khiin::proto;

constexpr auto kAsyncRWFlag = TF_ES_ASYNCDONTCARE | TF_ES_READWRITE;
constexpr auto kSyncRWFlag = TF_ES_SYNC | TF_ES_READWRITE;

const std::vector<InputScope> kUnallowedInputScopes = {
    IS_EMAIL_USERNAME,
    IS_EMAIL_SMTPEMAILADDRESS,
    IS_DIGITS,
    IS_NUMBER,
    IS_PASSWORD,
    IS_TELEPHONE_FULLTELEPHONENUMBER,
    IS_TELEPHONE_COUNTRYCODE,
    IS_TELEPHONE_AREACODE,
    IS_TELEPHONE_LOCALNUMBER,
    IS_TIME_FULLTIME,
    IS_TIME_HOUR,
    IS_TIME_MINORSEC,
};

void HandleCommitImpl(TfEditCookie ec, TextService *service, ITfContext *context, Command *command) {
    auto &res = command->response();
    auto comp = service->composition_mgr();

    if (res.has_preedit()) {
        auto preedit = CompositionUtil::WidenPreedit(res.preedit());
        comp->CommitComposition(ec, context, std::move(preedit));
    } else {
        comp->CommitComposition(ec, context);
    }

    service->candidate_ui()->Hide();
}

void HandleDoCompositionImpl(TfEditCookie ec, TextService *service, ITfContext *context, Command *command) {
    KHIIN_DEBUG("CMD_SEND_KEY");
    auto preedit = CompositionUtil::WidenPreedit(command->response().preedit());
    service->composition_mgr()->DoComposition(ec, context, preedit);
}

void HandleUpdateCandidatesImpl(TfEditCookie ec, TextService *service, ITfContext *context, Command *command) {
    WINRT_ASSERT(command != nullptr);
    WINRT_ASSERT(service != nullptr);
    WINRT_ASSERT(context != nullptr);

    auto ui = service->candidate_ui();
    auto comp = service->composition_mgr();

    if (!command->has_response()) {
        ui->Hide();
        return;
    }

    auto &res = command->response();

    if (res.candidate_list().candidates().size() > 0) {
        auto rect = CompositionUtil::TextPosition(ec, context, res.preedit().focused_caret());
        ui->Update(context, res.edit_state(), res.candidate_list(), rect);
        ui->Show();
    } else {
        ui->Hide();
    }
}

void HandleFocusChangeImpl(TfEditCookie ec, TextService *service, ITfContext *context) {
    KHIIN_DEBUG("checking input scopes");
    auto input_scopes = CompositionUtil::InputScopes(ec, context);

    if (input_scopes.empty()) {
        KHIIN_DEBUG("no input scopes");
        return;
    }

    auto keyboard_enabled = true;

    auto begin = kUnallowedInputScopes.cbegin();
    auto end = kUnallowedInputScopes.cend();

    for (auto const &scope : input_scopes) {
        if (std::find(begin, end, scope) != end) {
            keyboard_enabled = false;
            break;
        }
    }

    if (keyboard_enabled) {
        KHIIN_DEBUG("input enabled");
        service->SetLocked(false);
    } else {
        service->SetLocked(true);
    }
}

void HandleCommit(TextService *service, ITfContext *context, Command *command) {
    EditSession::ReadWriteSync(service, context, [&](TfEditCookie cookie) {
        HandleCommitImpl(cookie, service, context, command);
    });
}

void HandleDoComposition(TextService *service, ITfContext *context, Command *command) {
    EditSession::ReadWriteSync(service, context, [&](TfEditCookie cookie) {
        HandleDoCompositionImpl(cookie, service, context, command);
    });
}

void HandleUpdateCandidates(TextService *service, ITfContext *context, Command *command) {
    EditSession::ReadWriteSync(service, context, [&](TfEditCookie cookie) {
        HandleUpdateCandidatesImpl(cookie, service, context, command);
    });
}

/**
 * A simple helper class for obtaining the TfEditCookie (session cookie)
 * All operations that require an edit cookie should pass a callback receiving
 * the cookie into this class, which may be the be called with
 * |ITfContext::RequestEditSession|.
 *
 * Note that some operations must be performed with separate tokens, or they
 * will fail in certain cases. For example, requesting the screen coordinates
 * of composing text fails in some applications if the composition session
 * is not yet completed. Therefore, composition and requesting screen coordinates
 * must be done in separate sessions.
 */
struct CallbackEditSession : implements<CallbackEditSession, ITfEditSession> {
    CallbackEditSession(EditSession::CallbackFn callback) : callback(callback) {}
    STDMETHODIMP DoEditSession(TfEditCookie ec) override {
        TRY_FOR_HRESULT;
        callback(ec);
        CATCH_FOR_HRESULT;
    }
    EditSession::CallbackFn callback;
};

} // namespace

void EditSession::HandleFocusChange(TextService *service, ITfContext *context) {
    ReadWriteSync(service, context, [&](TfEditCookie cookie) {
        HandleFocusChangeImpl(cookie, service, context);
    });
}

void EditSession::HandleAction(TextService *service, ITfContext *context, Command *command) {
    switch (command->request().type()) {
    case CMD_SEND_KEY:
        [[fallthrough]];
    case CMD_FOCUS_CANDIDATE:
        [[fallthrough]];
    case CMD_SELECT_CANDIDATE:
        HandleDoComposition(service, context, command);
        HandleUpdateCandidates(service, context, command);
        return;
    case CMD_COMMIT:
        HandleCommit(service, context, command);
        return;
    default:
        return;
    }
}

void EditSession::ReadWriteSync(TextService *service, ITfContext *context, EditSession::CallbackFn callback) {
    auto ses = make_self<CallbackEditSession>(callback);
    auto ses_hr = E_FAIL;
    auto hr = context->RequestEditSession(service->client_id(), ses.get(), kSyncRWFlag, &ses_hr);
    CHECK_HRESULT(hr);
    CHECK_HRESULT(ses_hr);
}

} // namespace khiin::win32::tip
