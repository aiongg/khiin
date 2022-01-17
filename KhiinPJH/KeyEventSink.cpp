#include "pch.h"

#include "KeyEventSink.h"

#include "EditSession.h"
#include "common.h"

namespace Khiin {

enum class KeyEventSink::KeyAction { Test, Input };

KeyEventSink::~KeyEventSink() {
    uninit();
}

HRESULT KeyEventSink::init(TextService *pTextService, _In_ CandidateListUI *pCandidateListUI,
                           _In_ TextEngine *pEngine) {
    service.copy_from(pTextService);
    threadMgr.copy_from(service->threadMgr());
    compositionMgr.copy_from(cast_as<CompositionMgr>(service->compositionMgr()));
    keystrokeMgr = threadMgr.as<ITfKeystrokeMgr>();
    engine.copy_from(pEngine);
    candidateListUI.copy_from(pCandidateListUI);

    auto hr = keystrokeMgr->AdviseKeyEventSink(service->clientId(), this, TRUE);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT KeyEventSink::uninit() {
    WINRT_ASSERT(keystrokeMgr != nullptr);

    auto hr = keystrokeMgr->UnadviseKeyEventSink(service->clientId());
    CHECK_RETURN_HRESULT(hr);

    engine = nullptr;
    candidateListUI = nullptr;
    threadMgr = nullptr;
    keystrokeMgr = nullptr;
    compositionMgr = nullptr;
    service = nullptr;

    return S_OK;
}

HRESULT KeyEventSink::onTestKey(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(pContext);
    WINRT_ASSERT(compositionMgr);

    auto hr = E_FAIL;
    if (!compositionMgr->composing()) {
        hr = engine->clear();
        CHECK_RETURN_HRESULT(hr);
    }

    hr = engine->onTestKey(wParam, pfEaten);
    CHECK_RETURN_HRESULT(hr);

    if (!*pfEaten) {
        hr = engine->clear();
        CHECK_RETURN_HRESULT(hr);

        hr = compositionMgr->endComposition();
        CHECK_RETURN_HRESULT(hr);
    }

    return S_OK;
}

HRESULT KeyEventSink::onKey(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    auto hr = E_FAIL;
    hr = onTestKey(pContext, wParam, lParam, pfEaten);
    CHECK_RETURN_HRESULT(hr);

    std::string output;
    hr = engine->onKey(wParam, &output);
    CHECK_RETURN_HRESULT(hr);

    hr = compositionMgr->doComposition(pContext, output);
    CHECK_RETURN_HRESULT(hr);

    std::vector<std::string> candidates;
    hr = engine->getCandidates(&candidates);

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

    auto docMgr = winrt::com_ptr<ITfDocumentMgr>();
    hr = threadMgr->GetFocus(docMgr.put());
    CHECK_RETURN_HRESULT(hr);

    auto ctx = winrt::com_ptr<ITfContext>();
    hr = docMgr->GetTop(ctx.put());
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    D(__FUNCTIONW__);
    auto hr = onTestKey(pContext, wParam, lParam, pfEaten);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    return E_NOTIMPL;
}
STDMETHODIMP KeyEventSink::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    D(__FUNCTIONW__, " (W=", wParam, ")");

    auto hr = onKey(pContext, wParam, lParam, pfEaten);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) {
    return E_NOTIMPL;
}

STDMETHODIMP KeyEventSink::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten) {
    return E_NOTIMPL;
}

} // namespace Khiin
