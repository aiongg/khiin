#include "pch.h"

#include "proto/proto.h"

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "EditSession.h"
#include "EngineController.h"
#include "Guids.h"
#include "TextService.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;
using namespace khiin::proto;

RECT ParentWindowTopLeft(ITfContextView *view) {
    RECT rect = {};
    HWND parent_hwnd = NULL;
    check_hresult(view->GetWnd(&parent_hwnd));

    if (parent_hwnd == NULL) {
        parent_hwnd = ::GetFocus();
    }

    auto found = ::GetWindowRect(parent_hwnd, &rect);

    if (found != 0) {
        return RECT{rect.left, rect.top, rect.left + 1, rect.top + 1};
    }

    return rect;
}

HRESULT TextExtFromRange(TfEditCookie cookie, ITfContextView *view, ITfRange *range, RECT &rc) {
    LONG shifted = 0;
    range->ShiftStart(cookie, 0, &shifted, nullptr);
    range->Collapse(cookie, TF_ANCHOR_START);
    RECT rect{};
    BOOL clipped = FALSE;
    auto hr = view->GetTextExt(cookie, range, &rect, &clipped);
    rc = rect;
    return hr;
}

com_ptr<ITfRange> GetDefaultSelectionRange(TfEditCookie cookie, ITfContext *context) {
    TF_SELECTION selection = {};
    ULONG fetched = 0;
    check_hresult(context->GetSelection(cookie, TF_DEFAULT_SELECTION, 1, &selection, &fetched));

    auto range = com_ptr<ITfRange>();
    check_hresult(selection.range->Clone(range.put()));
    selection.range->Release();
    return range;
}

RECT GetEditRect(TfEditCookie ec, CompositionMgr *composition_mgr, ITfContext *context) {
    RECT rect = {};
    HRESULT hr = E_FAIL;
    auto view = winrt::com_ptr<ITfContextView>();
    check_hresult(context->GetActiveView(view.put()));

    if (composition_mgr) {
        auto range = composition_mgr->GetTextRange(ec);
        hr = TextExtFromRange(ec, view.get(), range.get(), rect);
        KHIIN_DEBUG("From composition range {} {} {} {}", rect.left, rect.top, rect.right, rect.bottom);
        CHECK_HRESULT(hr);
        if (hr == S_OK) {
            return rect;
        }
    }

    // Try default selection range
    auto range = GetDefaultSelectionRange(ec, context);
    hr = TextExtFromRange(ec, view.get(), range.get(), rect);
    KHIIN_DEBUG("From default selection {} {} {} {}", rect.left, rect.top, rect.right, rect.bottom);
    CHECK_HRESULT(hr);
    if (hr == S_OK) {
        return rect;
    }

    return ParentWindowTopLeft(view.get());
}

std::vector<InputScope> GetInputScopes(TfEditCookie cookie, ITfContext *context) {
    auto default_range = GetDefaultSelectionRange(cookie, context);
    auto prop = com_ptr<ITfReadOnlyProperty>();
    check_hresult(context->GetAppProperty(guids::kPropInputScope, prop.put()));
    VARIANT var{};
    ::VariantInit(&var);
    check_hresult(prop->GetValue(cookie, default_range.get(), &var));

    if (var.vt != VT_UNKNOWN) {
        return std::vector<InputScope>();
    }

    auto unk = com_ptr<IUnknown>();
    unk.copy_from(var.punkVal);
    auto input_scope = unk.as<ITfInputScope>();
    InputScope *buffer = nullptr;
    UINT num_scopes = 0;
    check_hresult(input_scope->GetInputScopes(&buffer, &num_scopes));
    auto ret = std::vector<InputScope>();
    ret.assign(buffer, buffer + num_scopes);
    ::CoTaskMemFree(buffer);
    return ret;
}

constexpr auto kAsyncRWFlag = TF_ES_ASYNCDONTCARE | TF_ES_READWRITE;
constexpr auto kSyncRWFlag = TF_ES_SYNC | TF_ES_READWRITE;

struct HandleFocusEditSession : implements<HandleFocusEditSession, ITfEditSession> {
    HandleFocusEditSession(TextService *service, ITfContext *context) {
        m_service.copy_from(service);
        m_context.copy_from(context);
    }

    ~HandleFocusEditSession() = default;

  private:
    //+---------------------------------------------------------------------------
    //
    // ITfEditSession
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP DoEditSession(TfEditCookie cookie) override {
        TRY_FOR_HRESULT;
        KHIIN_DEBUG("checking input scopes");
        auto input_scopes = GetInputScopes(cookie, m_context.get());

        if (input_scopes.empty()) {
            KHIIN_DEBUG("no input scopes");
            return S_OK;
        }

        auto keyboard_enabled = true;

        for (auto const &scope : input_scopes) {
            switch (scope) {
            case IS_URL:
                [[fallthrough]];
            case IS_EMAIL_USERNAME:
                [[fallthrough]];
            case IS_EMAIL_SMTPEMAILADDRESS:
                [[fallthrough]];
            case IS_DIGITS:
                [[fallthrough]];
            case IS_NUMBER:
                [[fallthrough]];
            case IS_PASSWORD:
                [[fallthrough]];
            case IS_TELEPHONE_FULLTELEPHONENUMBER:
                [[fallthrough]];
            case IS_TELEPHONE_COUNTRYCODE:
                [[fallthrough]];
            case IS_TELEPHONE_AREACODE:
                [[fallthrough]];
            case IS_TELEPHONE_LOCALNUMBER:
                [[fallthrough]];
            case IS_TIME_FULLTIME:
                [[fallthrough]];
            case IS_TIME_HOUR:
                [[fallthrough]];
            case IS_TIME_MINORSEC:
                KHIIN_DEBUG("input disabled");
                keyboard_enabled = false;
                break;
            }

            if (!keyboard_enabled) {
                break;
            }
        }

        m_service->TipOpenClose(keyboard_enabled);
        CATCH_FOR_HRESULT;
    }

    com_ptr<TextService> m_service;
    com_ptr<ITfContext> m_context;
};

struct HandleActionEditSession : winrt::implements<HandleActionEditSession, ITfEditSession> {
    HandleActionEditSession(TextService *service, ITfContext *context, Command *command) : command(command) {
        KHIIN_TRACE("");
        m_service.copy_from(service);
        m_context.copy_from(context);
    }
    ~HandleActionEditSession() = default;

  private:
    //+---------------------------------------------------------------------------
    //
    // ITfEditSession
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP DoEditSession(TfEditCookie ec) override {
        KHIIN_TRACE("");
        TRY_FOR_HRESULT;
        auto composition_mgr = m_service->composition_mgr();
        auto candidate_ui = m_service->candidate_ui();
        bool composing = composition_mgr->composing();
        bool showing = candidate_ui->Showing();
        auto cmd_type = command->request().type();
        auto &response = command->response();

        if (response.error() == ErrorCode::FAIL) {
            KHIIN_DEBUG("FAIL");
            composition_mgr->CommitComposition(ec, m_context.get());
            candidate_ui->Hide();
        } else if (cmd_type == CMD_COMMIT) {
            KHIIN_DEBUG("CMD_COMMIT");
            if (response.preedit().segments().size() == 0) {
                composition_mgr->CommitComposition(ec, m_context.get());
            } else {
                composition_mgr->CommitComposition(ec, m_context.get(), response.preedit());
            }
            if (showing) {
                candidate_ui->Hide();
            }
        } else if (cmd_type == CMD_SEND_KEY || cmd_type == CMD_SELECT_CANDIDATE || cmd_type == CMD_FOCUS_CANDIDATE) {
            KHIIN_DEBUG("CMD_SEND_KEY");
            composition_mgr->DoComposition(ec, m_context.get(), response.preedit());

            if (response.candidate_list().candidates().size() > 0) {
                candidate_ui->Update(m_context.get(), response.edit_state(), response.candidate_list(),
                                     GetEditRect(ec, composition_mgr.get(), m_context.get()));
                if (!showing) {
                    candidate_ui->Show();
                }
            } else {
                candidate_ui->Hide();
            }
        }

        return S_OK;
        CATCH_FOR_HRESULT;
    }

    winrt::com_ptr<TextService> m_service = nullptr;
    winrt::com_ptr<ITfContext> m_context = nullptr;
    Command *command;
};

} // namespace

void EditSession::HandleFocusChange(TextService *service, ITfDocumentMgr *docmgr) {
    if (docmgr == nullptr) {
        return;
    }
    auto context = com_ptr<ITfContext>();
    check_hresult(docmgr->GetBase(context.put()));
    auto session = make_self<HandleFocusEditSession>(service, context.get());
    auto ses_hr = E_FAIL;
    check_hresult(context->RequestEditSession(service->clientId(), session.get(), kAsyncRWFlag, &ses_hr));
    check_hresult(ses_hr);
}

void EditSession::HandleAction(TextService *service, ITfContext *context, Command *command) {
    auto session = make_self<HandleActionEditSession>(service, context, command);
    auto ses_hr = E_FAIL;
    check_hresult(context->RequestEditSession(service->clientId(), session.get(), kAsyncRWFlag, &ses_hr));
    check_hresult(ses_hr);
}

} // namespace khiin::win32::tip
