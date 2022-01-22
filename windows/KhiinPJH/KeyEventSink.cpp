#include "pch.h"

#include "KeyEventSink.h"

#include "EditSession.h"
#include "common.h"

namespace khiin::win32 {

enum class KeyEventSink::KeyAction { Test, Input };

KeyEventSink::~KeyEventSink() {
    Deactivate();
}

void KeyEventSink::Activate(TextService *pTextService) {
    service.copy_from(pTextService);
    threadMgr.copy_from(service->threadMgr());
    compositionMgr.copy_from(cast_as<CompositionMgr>(service->compositionMgr()));
    keystrokeMgr = threadMgr.as<ITfKeystrokeMgr>();

    winrt::check_hresult(keystrokeMgr->AdviseKeyEventSink(service->clientId(), this, TRUE));
}

void KeyEventSink::Deactivate() {
    D(__FUNCTIONW__);
    if (keystrokeMgr) {
        winrt::check_hresult(keystrokeMgr->UnadviseKeyEventSink(service->clientId()));
    }

    threadMgr = nullptr;
    keystrokeMgr = nullptr;
    compositionMgr = nullptr;
    service = nullptr;
}

void KeyEventSink::TestKey(ITfContext *pContext, KeyEvent keyEvent, BOOL *pfEaten) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(pContext);
    WINRT_ASSERT(compositionMgr);

    if (!compositionMgr->composing()) {
        service->engine()->Reset();
    }

    auto action = service->engine()->TestKey(keyEvent);

    *pfEaten = action.consumed;
    EditSession::HandleAction(service.get(), pContext, std::move(action));
}

void KeyEventSink::HandleKey(ITfContext *pContext, KeyEvent keyEvent, BOOL *pfEaten) {
    TestKey(pContext, keyEvent, pfEaten);

    if (!*pfEaten) {
        return;
    }

    auto action = service->engine()->OnKey(keyEvent);
    EditSession::HandleAction(service.get(), pContext, std::move(action));
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
