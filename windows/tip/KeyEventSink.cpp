#include "pch.h"

#include "KeyEventSink.h"

#include "EditSession.h"
#include "EngineController.h"
#include "KeyEvent.h"
#include "proto/proto.h"

namespace khiin::win32::tip {
namespace {

bool TestPageKeyForCandidateUI(CandidateListUI *cand_ui, win32::KeyEvent const &key_event) {
    if (cand_ui->Selecting()) {
        auto key = key_event.keyCode();
        if (cand_ui->MultiColumn() && (key == VK_LEFT || key == VK_RIGHT)) {
            return true;
        }

        if (cand_ui->PageCount() > 1 && (key == VK_NEXT || key == VK_PRIOR)) {
            return true;
        }
    }

    return false;
}

bool TestQuickSelectForCandidateUI(CandidateListUI *cand_ui, win32::KeyEvent const &key_event) {
    if (cand_ui->Selecting() && key_event.ascii()) {
        auto qs = key_event.ascii() - '0';
        auto qs_max = cand_ui->MaxQuickSelect();
        return 1 <= qs && qs <= qs_max;
    }

    return false;
}

bool TestKeyForCandidateUI(CandidateListUI *cand_ui, win32::KeyEvent const &key_event) {
    return TestPageKeyForCandidateUI(cand_ui, key_event) || TestQuickSelectForCandidateUI(cand_ui, key_event);
}

bool HandleQuickSelect(TextService *service, ITfContext *context, CandidateListUI *cand_ui,
                       win32::KeyEvent const &key_event) {
    auto id = cand_ui->QuickSelect(key_event.ascii() - '0' - 1);

    if (id >= 0) {
        auto command = service->engine()->SelectCandidate(id);
        EditSession::HandleAction(service, context, command);
        return true;
    }

    return false;
}

bool HandleCandidatePage(TextService *service, ITfContext *context, CandidateListUI *cand_ui,
                         win32::KeyEvent const &key_event) {

    auto id = -1;

    if (key_event.keyCode() == VK_NEXT) {
        id = cand_ui->RotateNext();
    } else if (key_event.keyCode() == VK_PRIOR) {
        id = cand_ui->RotatePrev();
    }

    if (id >= 0) {
        auto command = service->engine()->FocusCandidate(id);
        EditSession::HandleAction(service, context, std::move(command));
        return true;
    }

    // TODO: Handle left/right in multi-column grid

    return false;
}

void HandleKeyBasic(TextService *service, ITfContext *context, win32::KeyEvent const &key_event) {
    auto command = service->engine()->OnKey(key_event);
    EditSession::HandleAction(service, context, std::move(command));
}

} // namespace

using namespace proto;

enum class KeyEventSink::KeyAction { Test, Input };

KeyEventSink::~KeyEventSink() {
    Deactivate();
}

void KeyEventSink::Activate(TextService *pTextService) {
    service.copy_from(pTextService);
    thread_mgr.copy_from(service->thread_mgr());
    composition_mgr.copy_from(cast_as<CompositionMgr>(service->composition_mgr()));
    keystrokeMgr = thread_mgr.as<ITfKeystrokeMgr>();

    winrt::check_hresult(keystrokeMgr->AdviseKeyEventSink(service->clientId(), this, TRUE));
}

void KeyEventSink::Deactivate() {
    KHIIN_TRACE("");
    if (keystrokeMgr) {
        winrt::check_hresult(keystrokeMgr->UnadviseKeyEventSink(service->clientId()));
    }

    thread_mgr = nullptr;
    keystrokeMgr = nullptr;
    composition_mgr = nullptr;
    service = nullptr;
}

void KeyEventSink::TestKey(ITfContext *pContext, win32::KeyEvent keyEvent, BOOL *pfEaten) {
    KHIIN_TRACE("");
    WINRT_ASSERT(pContext);
    WINRT_ASSERT(composition_mgr);

    if (TestKeyForCandidateUI(service->candidate_ui(), keyEvent)) {
        *pfEaten = TRUE;
        return;
    }

    if (!composition_mgr->composing()) {
        service->engine()->Reset();
    }

    auto command = service->engine()->TestKey(keyEvent);

    if (command->response().consumable()) {
        *pfEaten = TRUE;
        return;
    }

    EditSession::HandleAction(service.get(), pContext, std::move(command));
}

void KeyEventSink::HandleKey(ITfContext *pContext, win32::KeyEvent keyEvent, BOOL *pfEaten) {
    if (TestQuickSelectForCandidateUI(service->candidate_ui(), keyEvent)) {
        HandleQuickSelect(service.get(), pContext, service->candidate_ui(), keyEvent);
        *pfEaten = TRUE;
        return;
    } else if (TestPageKeyForCandidateUI(service->candidate_ui(), keyEvent)) {
        HandleCandidatePage(service.get(), pContext, service->candidate_ui(), keyEvent);
        *pfEaten = TRUE;
        return;
    }

    TestKey(pContext, keyEvent, pfEaten);

    if (!*pfEaten) {
        return;
    }

    HandleKeyBasic(service.get(), pContext, keyEvent);
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink
//
//----------------------------------------------------------------------------

STDMETHODIMP KeyEventSink::OnSetFocus(BOOL fForeground) {
    TRY_FOR_HRESULT;
    if (!fForeground) {
        return S_OK;
    }

    return E_NOTIMPL;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP KeyEventSink::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    TRY_FOR_HRESULT;
    KHIIN_TRACE("");

    *pfEaten = false;
    auto keyEvent = win32::KeyEvent(WM_KEYDOWN, wParam, lParam);
    TestKey(pContext, keyEvent, pfEaten);

    return S_OK;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP KeyEventSink::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    TRY_FOR_HRESULT;

    auto keyEvent = win32::KeyEvent(WM_KEYUP, wParam, lParam);

    return E_NOTIMPL;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP KeyEventSink::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    TRY_FOR_HRESULT;
    KHIIN_TRACE("");

    *pfEaten = false;
    auto keyEvent = win32::KeyEvent(WM_KEYDOWN, wParam, lParam);
    HandleKey(pContext, keyEvent, pfEaten);

    return S_OK;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP KeyEventSink::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    TRY_FOR_HRESULT;
    auto keyEvent = win32::KeyEvent(WM_KEYUP, wParam, lParam);

    return E_NOTIMPL;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP KeyEventSink::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten) {
    TRY_FOR_HRESULT;
    return E_NOTIMPL;
    CATCH_FOR_HRESULT;
}

} // namespace khiin::win32::tip
