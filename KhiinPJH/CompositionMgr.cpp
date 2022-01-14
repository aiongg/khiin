#include "pch.h"

#include "CompositionMgr.h"

#include "EditSession.h"

namespace Khiin {

HRESULT CompositionMgr::init(TfClientId clientId) {
    D(__FUNCTIONW__);
    this->clientId = clientId;
    return S_OK;
}

HRESULT CompositionMgr::uninit() {
    D(__FUNCTIONW__);
    clientId = TF_CLIENTID_NULL;
    composition = nullptr;
    context = nullptr;
    return S_OK;
}

bool CompositionMgr::composing() {
    return composition != nullptr;
}

HRESULT CompositionMgr::setText(TfEditCookie cookie, ITfContext *pContext, std::string text) {
    D(__FUNCTIONW__);
    HRESULT hr = E_FAIL;

    if (!composing()) {
        hr = requestEditSession(pContext);
        CHECK_RETURN_HRESULT(hr);
    }

    auto range = winrt::com_ptr<ITfRange>();
    hr = composition->GetRange(range.put());
    CHECK_RETURN_HRESULT(hr);

    auto wstr = std::wstring(text.cbegin(), text.cend());
    auto len = wstr.size();
    hr = range->SetText(cookie, TF_ST_CORRECTION, &wstr[0], static_cast<LONG>(len));
    CHECK_RETURN_HRESULT(hr);

    range = nullptr;
    hr = composition->GetRange(range.put());
    CHECK_RETURN_HRESULT(hr);

    auto clone = winrt::com_ptr<ITfRange>();
    hr = range->Clone(clone.put());
    CHECK_RETURN_HRESULT(hr);

    hr = clone->Collapse(cookie, TF_ANCHOR_END);
    CHECK_RETURN_HRESULT(hr);

    TF_SELECTION sel{};
    sel.range = clone.get();
    sel.style.ase = TF_AE_NONE;
    sel.style.fInterimChar = FALSE;
    hr = pContext->SetSelection(cookie, 1, &sel);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT CompositionMgr::requestEditSession(ITfContext *pContext) {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    hr = EditSession::request(clientId, pContext, [&](TfEditCookie cookie) {
        auto hr = startComposition(cookie, pContext);
        CHECK_RETURN_HRESULT(hr);
        return S_OK;
    });
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT CompositionMgr::startComposition(TfEditCookie cookie, ITfContext *pContext) {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    auto compositionMgr = winrt::com_ptr<ITfCompositionSink>();
    hr = this->QueryInterface(__uuidof(ITfCompositionSink), compositionMgr.put_void());
    CHECK_RETURN_HRESULT(hr);

    auto context = winrt::com_ptr<ITfContext>();
    context.copy_from(pContext);

    auto insertAtSelection = context.as<ITfInsertAtSelection>();
    auto contextComposition = context.as<ITfContextComposition>();
    auto range = winrt::com_ptr<ITfRange>();
    hr = insertAtSelection->InsertTextAtSelection(cookie, TF_IAS_QUERYONLY, NULL, 0, range.put());
    CHECK_RETURN_HRESULT(hr);

    auto composition = winrt::com_ptr<ITfComposition>();
    hr = contextComposition->StartComposition(cookie, range.get(), compositionMgr.get(), composition.put());
    CHECK_RETURN_HRESULT(hr);

    this->composition = nullptr;
    this->composition = composition;

    TF_SELECTION sel{};
    sel.range = range.get();
    sel.style.ase = TF_AE_NONE;
    sel.style.fInterimChar = FALSE;
    hr = context->SetSelection(cookie, 1, &sel);
    CHECK_RETURN_HRESULT(hr);

    this->context = nullptr;
    this->context = context;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCompositionSink
//
//----------------------------------------------------------------------------

STDMETHODIMP CompositionMgr::OnCompositionTerminated(TfEditCookie cookie, ITfComposition *pComposition) {
    D(__FUNCTIONW__);
    // auto hr = E_FAIL;
    // hr = composition->EndComposition(cookie);
    // CHECK_RETURN_HRESULT(hr);

    composition = nullptr;
    context = nullptr;
    return S_OK;
}

} // namespace Khiin
