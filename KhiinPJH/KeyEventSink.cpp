#include "pch.h"

#include "KeyEventSink.h"

#include "EditSession.h"
#include "common.h"

namespace Khiin {

enum class KeyEventSink::KeyAction { Test, Input };

KeyEventSink::~KeyEventSink() {
    uninit();
}

HRESULT KeyEventSink::init(TextService *pTextService) {
    service.copy_from(pTextService);
    threadMgr.copy_from(service->threadMgr());
    compositionMgr.copy_from(cast_as<CompositionMgr>(service->compositionMgr()));
    keystrokeMgr = threadMgr.as<ITfKeystrokeMgr>();

    auto hr = keystrokeMgr->AdviseKeyEventSink(service->clientId(), this, TRUE);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT KeyEventSink::uninit() {
    WINRT_ASSERT(keystrokeMgr != nullptr);

    auto hr = keystrokeMgr->UnadviseKeyEventSink(service->clientId());
    CHECK_RETURN_HRESULT(hr);

    threadMgr = nullptr;
    keystrokeMgr = nullptr;
    compositionMgr = nullptr;
    service = nullptr;

    return S_OK;
}

HRESULT KeyEventSink::onTestKey(ITfContext *pContext, KeyEvent keyEvent, BOOL *pfEaten) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(pContext);
    WINRT_ASSERT(compositionMgr);

    auto hr = E_FAIL;
    auto engine = service->engine();

    if (!compositionMgr->composing()) {
        hr = engine->clear();
        CHECK_RETURN_HRESULT(hr);
    }

    hr = engine->onTestKey(keyEvent, pfEaten);
    CHECK_RETURN_HRESULT(hr);

    if (*pfEaten) {
        return S_OK;
    }

    hr = engine->clear();
    CHECK_RETURN_HRESULT(hr);

    if (compositionMgr->composing()) {
        auto action = Action();
        action.msg = Message::CommitText;
        hr = EditSession::handleAction(service.get(), pContext, std::move(action));
        CHECK_RETURN_HRESULT(hr);
    }

    return S_OK;
}

HRESULT KeyEventSink::onKey(ITfContext *pContext, KeyEvent keyEvent, BOOL *pfEaten) {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    hr = onTestKey(pContext, keyEvent, pfEaten);
    CHECK_RETURN_HRESULT(hr);

    if (!*pfEaten) {
        return S_OK;
    }

    hr = service->engine()->onKey(keyEvent);
    CHECK_RETURN_HRESULT(hr);

    auto action = Action();

    if (compositionMgr->composing()) {
        action.msg = Message::UpdateComposition;
    } else {
        action.msg = Message::StartComposition;
    }

    action.text = service->engine()->buffer();
    action.candidates = service->engine()->candidates();

    hr = EditSession::handleAction(service.get(), pContext, std::move(action));
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink
//
//----------------------------------------------------------------------------

STDMETHODIMP KeyEventSink::OnSetFocus(BOOL fForeground) {
    D(__FUNCTIONW__);
    if (!fForeground) {
        return S_OK;
    }

    auto hr = E_FAIL;

    //auto docMgr = winrt::com_ptr<ITfDocumentMgr>();
    //hr = threadMgr->GetFocus(docMgr.put());
    //CHECK_RETURN_HRESULT(hr);
    //
    //auto ctx = winrt::com_ptr<ITfContext>();
    //hr = docMgr->GetTop(ctx.put());
    //CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    D(__FUNCTIONW__);

    auto keyEvent = KeyEvent(WM_KEYDOWN, wParam, lParam);
    auto hr = onTestKey(pContext, keyEvent, pfEaten);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    auto keyEvent = KeyEvent(WM_KEYUP, wParam, lParam);

    return E_NOTIMPL;
}
STDMETHODIMP KeyEventSink::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    D(__FUNCTIONW__, " (W=", wParam, ")");

    auto keyEvent = KeyEvent(WM_KEYDOWN, wParam, lParam);
    auto hr = onKey(pContext, keyEvent, pfEaten);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    auto keyEvent = KeyEvent(WM_KEYUP, wParam, lParam);

    return E_NOTIMPL;
}

STDMETHODIMP KeyEventSink::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten) {
    return E_NOTIMPL;
}

} // namespace Khiin
