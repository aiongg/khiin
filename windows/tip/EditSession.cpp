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
        comp->CommitComposition(ec, context, res.preedit());
    } else {
        comp->CommitComposition(ec, context);
    }

    service->candidate_ui()->Hide();
}

void HandleDoCompositionImpl(TfEditCookie ec, TextService *service, ITfContext *context, Command *command) {
    KHIIN_DEBUG("CMD_SEND_KEY");
    service->composition_mgr()->DoComposition(ec, context, command->response().preedit());
}

void HandleUpdateCandidatesImpl(TfEditCookie ec, TextService *service, ITfContext *context, Command *command) {
    auto &res = command->response();
    auto ui = service->candidate_ui();
    auto comp = service->composition_mgr();

    if (res.has_candidate_list() && res.candidate_list().candidates_size() > 0) {
        auto rect = CompositionUtil::TextPosition(ec, context);
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

struct CallbackEditSession : implements<CallbackEditSession, ITfEditSession> {
    CallbackEditSession(EditSession::CallbackFn cb) : callback(cb) {}
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
    case CMD_COMMIT:
        HandleCommit(service, context, command);
        return;
    case CMD_SEND_KEY:
        [[fallthrough]];
    case CMD_FOCUS_CANDIDATE:
        [[fallthrough]];
    case CMD_SELECT_CANDIDATE:
        HandleDoComposition(service, context, command);
        HandleUpdateCandidates(service, context, command);
        return;
    default:
        return;
    }
}

void EditSession::ReadWriteSync(TextService *service, ITfContext *context, EditSession::CallbackFn callback) {
    auto ses = make_self<CallbackEditSession>(callback);
    auto ses_hr = E_FAIL;
    auto hr = context->RequestEditSession(service->clientId(), ses.get(), kSyncRWFlag, &ses_hr);
    CHECK_HRESULT(hr);
    CHECK_HRESULT(ses_hr);
}

} // namespace khiin::win32::tip
