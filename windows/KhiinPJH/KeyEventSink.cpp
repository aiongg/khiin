#include "pch.h"

#include "KeyEventSink.h"

#include "EngineController.h"
#include "EditSession.h"
#include "common.h"

namespace khiin::win32 {
    using namespace messages;

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
    D(__FUNCTIONW__);
    if (keystrokeMgr) {
        winrt::check_hresult(keystrokeMgr->UnadviseKeyEventSink(service->clientId()));
    }

    thread_mgr = nullptr;
    keystrokeMgr = nullptr;
    composition_mgr = nullptr;
    service = nullptr;
}

void KeyEventSink::TestKey(ITfContext *pContext, KeyEvent keyEvent, BOOL *pfEaten) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(pContext);
    WINRT_ASSERT(composition_mgr);

    if (!composition_mgr->composing()) {
        service->engine()->Reset();
    }

    auto command = service->engine()->TestKey(keyEvent);

    if (command.output().consumable()) {
        *pfEaten = true;
        return;
    }

    EditSession::HandleAction(service.get(), pContext, std::move(command));
}

void KeyEventSink::HandleKey(ITfContext *pContext, KeyEvent keyEvent, BOOL *pfEaten) {
    TestKey(pContext, keyEvent, pfEaten);

    if (!*pfEaten) {
        return;
    }

    auto command = service->engine()->OnKey(keyEvent);

    if (command.output().consumable()) {
        *pfEaten = true;
    }

    EditSession::HandleAction(service.get(), pContext, std::move(command));
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
    D(__FUNCTIONW__);

    *pfEaten = false;
    auto keyEvent = KeyEvent(WM_KEYDOWN, wParam, lParam);
    TestKey(pContext, keyEvent, pfEaten);

    return S_OK;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP KeyEventSink::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    TRY_FOR_HRESULT;

    auto keyEvent = KeyEvent(WM_KEYUP, wParam, lParam);

    return E_NOTIMPL;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP KeyEventSink::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    TRY_FOR_HRESULT;
    D(__FUNCTIONW__, " (W=", wParam, ")");

    *pfEaten = false;
    auto keyEvent = KeyEvent(WM_KEYDOWN, wParam, lParam);
    HandleKey(pContext, keyEvent, pfEaten);

    return S_OK;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP KeyEventSink::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    TRY_FOR_HRESULT;
    auto keyEvent = KeyEvent(WM_KEYUP, wParam, lParam);

    return E_NOTIMPL;
    CATCH_FOR_HRESULT;
}

STDMETHODIMP KeyEventSink::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten) {
    TRY_FOR_HRESULT;
    return E_NOTIMPL;
    CATCH_FOR_HRESULT;
}

} // namespace khiin::win32
