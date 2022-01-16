#include "pch.h"

#include "CompositionMgr.h"

#include "DisplayAttributeInfoEnum.h"
#include "EditSession.h"

namespace Khiin {

HRESULT CompositionMgr::init(TfClientId clientId, DisplayAttributeInfoEnum *pDaiEnum) {
    D(__FUNCTIONW__);
    this->clientId = clientId;
    attributes.copy_from(pDaiEnum);

    auto hr = ::CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,
                                 categoryMgr.put_void());
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT CompositionMgr::uninit() {
    D(__FUNCTIONW__);
    clientId = TF_CLIENTID_NULL;
    attributes = nullptr;
    composition = nullptr;
    context = nullptr;
    return S_OK;
}

bool CompositionMgr::composing() {
    return bool(composition);
}

HRESULT CompositionMgr::startComposition(ITfContext *pContext) {
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
    WINRT_ASSERT(clientId != TF_CLIENTID_NULL);

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

HRESULT CompositionMgr::doComposition(ITfContext *pContext, std::string text) {
    auto hr = EditSession::request(clientId, pContext, [&](TfEditCookie cookie) {
        D(__FUNCTIONW__);
        auto hr = doComposition(cookie, pContext, text);
        CHECK_RETURN_HRESULT(hr);
        return S_OK;
    });
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT CompositionMgr::doComposition(TfEditCookie cookie, ITfContext *pContext, std::string text) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(clientId != TF_CLIENTID_NULL);
    HRESULT hr = E_FAIL;

    if (!composing()) {
        hr = startComposition(pContext);
        CHECK_RETURN_HRESULT(hr);
    }

    auto range = winrt::com_ptr<ITfRange>();
    hr = composition->GetRange(range.put());
    CHECK_RETURN_HRESULT(hr);

    auto wstr = std::wstring(text.cbegin(), text.cend());
    auto wstrlen = static_cast<LONG>(wstr.size());
    hr = range->SetText(cookie, TF_ST_CORRECTION, &wstr[0], wstrlen);
    CHECK_RETURN_HRESULT(hr);

    hr = applyDisplayAttribute(cookie, pContext, range.get(), AttributeIndex::Input);
    CHECK_RETURN_HRESULT(hr);

    hr = collapseCursorToEnd(cookie, pContext);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT CompositionMgr::endComposition() {
    WINRT_ASSERT(clientId != TF_CLIENTID_NULL);

    if (!context || !composing()) {
        return S_OK;
    }

    auto hr = EditSession::request(clientId, context.get(), [&](TfEditCookie cookie) {
        D(__FUNCTIONW__);
        auto hr = endComposition(cookie);
        CHECK_RETURN_HRESULT(hr);

        return S_OK;
    });
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT CompositionMgr::endComposition(TfEditCookie cookie) {
    D(__FUNCTIONW__);

    if (!composing()) {
        return S_OK;
    }

    auto hr = E_FAIL;
    auto range = winrt::com_ptr<ITfRange>();
    hr = composition->GetRange(range.put());
    CHECK_RETURN_HRESULT(hr);

    auto clone = winrt::com_ptr<ITfRange>();
    hr = range->Clone(clone.put());
    CHECK_RETURN_HRESULT(hr);

    hr = clone->Collapse(cookie, TF_ANCHOR_END);
    CHECK_RETURN_HRESULT(hr);

    auto sel = TF_SELECTION{};
    sel.range = clone.get();
    sel.style.ase = TF_AE_NONE;
    sel.style.fInterimChar = FALSE;
    hr = context->SetSelection(cookie, 1, &sel);
    CHECK_RETURN_HRESULT(hr);

    composition->EndComposition(cookie);
    composition = nullptr;

    return S_OK;
}

HRESULT CompositionMgr::setText(TfEditCookie cookie, std::string_view text) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(clientId != TF_CLIENTID_NULL);

    if (!composing()) {
        return E_ABORT;
    }

    HRESULT hr = E_FAIL;
    return S_OK;
}

HRESULT CompositionMgr::applyDisplayAttribute(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange,
                                              AttributeIndex index) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(clientId != TF_CLIENTID_NULL);

    if (!composing()) {
        return E_ABORT;
    }

    HRESULT hr = E_FAIL;
    auto attr = winrt::com_ptr<ITfDisplayAttributeInfo>();
    attributes->at(index, attr.put());

    GUID guid{};
    hr = attr->GetGUID(&guid);
    CHECK_RETURN_HRESULT(hr);

    auto guidstr = std::wstring(39, '?');
    ::StringFromGUID2(guid, &guidstr[0], 39);
    D("GUID: ", guidstr);

    TfGuidAtom atom = TF_INVALID_GUIDATOM;
    hr = categoryMgr->RegisterGUID(guid, &atom);
    CHECK_RETURN_HRESULT(hr);

    if (atom == TF_INVALID_GUIDATOM) {
        return E_INVALIDARG;
    }

    auto prop = winrt::com_ptr<ITfProperty>();
    hr = pContext->GetProperty(GUID_PROP_ATTRIBUTE, prop.put());
    CHECK_RETURN_HRESULT(hr);

    VARIANT var;
    ::VariantInit(&var);
    var.vt = VT_I4;
    var.lVal = atom;

    hr = prop->SetValue(cookie, pRange, &var);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT CompositionMgr::collapseCursorToEnd(TfEditCookie cookie, ITfContext *pContext) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(clientId != TF_CLIENTID_NULL);

    if (!composing()) {
        return E_ABORT;
    }

    auto hr = E_FAIL;
    auto clone = winrt::com_ptr<ITfRange>();
    auto range = winrt::com_ptr<ITfRange>();

    hr = composition->GetRange(range.put());
    CHECK_RETURN_HRESULT(hr);

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

//+---------------------------------------------------------------------------
//
// ITfCompositionSink
//
//----------------------------------------------------------------------------

STDMETHODIMP CompositionMgr::OnCompositionTerminated(TfEditCookie cookie, ITfComposition *pComposition) {
    D(__FUNCTIONW__);

    auto hr = endComposition(cookie);
    CHECK_RETURN_HRESULT(hr);

    composition = nullptr;
    context = nullptr;
    return S_OK;
}

} // namespace Khiin
