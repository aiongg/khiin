#include "pch.h"

#include "CompositionMgr.h"

#include "DisplayAttributeInfoEnum.h"
#include "EditSession.h"

#include "common.h"

namespace Khiin {

//+---------------------------------------------------------------------------
//
// Public methods
//
//----------------------------------------------------------------------------

CompositionMgr::~CompositionMgr() {
    uninit();
}

HRESULT CompositionMgr::init(TextService *pTextService) {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    service.copy_from(pTextService);
    attributes.copy_from(cast_as<DisplayAttributeInfoEnum>(service->displayAttrInfoEnum()));
    WINRT_ASSERT(attributes);

    hr = service->categoryMgr(categoryMgr.put());
    CHECK_RETURN_HRESULT(hr)
    return S_OK;
}

void CompositionMgr::clearComposition() {
    composition = nullptr;
}

HRESULT CompositionMgr::uninit() {
    D(__FUNCTIONW__);
    service = nullptr;
    attributes = nullptr;
    categoryMgr = nullptr;
    composition = nullptr;
    context = nullptr;
    return S_OK;
}

bool CompositionMgr::composing() {
    return bool(composition);
}

//+---------------------------------------------------------------------------
//
// Private methods
//
//----------------------------------------------------------------------------

HRESULT CompositionMgr::startComposition(TfEditCookie cookie, ITfContext *pContext) {
    D(__FUNCTIONW__);

    auto hr = E_FAIL;
    auto context = winrt::com_ptr<ITfContext>();
    context.copy_from(pContext);

    auto insertAtSelection = context.as<ITfInsertAtSelection>();
    auto contextComposition = context.as<ITfContextComposition>();

    auto range = winrt::com_ptr<ITfRange>();
    hr = insertAtSelection->InsertTextAtSelection(cookie, TF_IAS_QUERYONLY, NULL, 0, range.put());
    CHECK_RETURN_HRESULT(hr);

    auto composition = winrt::com_ptr<ITfComposition>();
    auto compositionSink = winrt::com_ptr<ITfCompositionSink>();
    service->compositionSink(pContext, compositionSink.put());
    hr = contextComposition->StartComposition(cookie, range.get(), compositionSink.get(), composition.put());
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

HRESULT CompositionMgr::doComposition(TfEditCookie cookie, ITfContext *pContext, std::string text) {
    D(__FUNCTIONW__);
    HRESULT hr = E_FAIL;

    if (!composing()) {
        hr = startComposition(cookie, pContext);
        CHECK_RETURN_HRESULT(hr);
    }

    auto range = winrt::com_ptr<ITfRange>();
    hr = composition->GetRange(range.put());
    CHECK_RETURN_HRESULT(hr);

    auto wstr = std::wstring(text.cbegin(), text.cend());
    auto wstrlen = static_cast<LONG>(wstr.size());
    hr = range->SetText(cookie, TF_ST_CORRECTION, &wstr[0], wstrlen);
    CHECK_RETURN_HRESULT(hr);

    hr = applyDisplayAttribute(cookie, pContext, range.get(), AttrInfoKey::Input);
    CHECK_RETURN_HRESULT(hr);

    hr = collapseCursorToEnd(cookie, pContext);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT CompositionMgr::endComposition(TfEditCookie cookie) {
    D(__FUNCTIONW__);

    if (!composing()) {
        return S_OK;
    }

    auto hr = E_FAIL;

    hr = collapseCursorToEnd(cookie, context.get());
    CHECK_RETURN_HRESULT(hr);

    hr = composition->EndComposition(cookie);
    CHECK_RETURN_HRESULT(hr);

    composition = nullptr;

    return S_OK;
}

HRESULT CompositionMgr::applyDisplayAttribute(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange,
                                              AttrInfoKey index) {
    D(__FUNCTIONW__);

    if (!composing()) {
        return E_ABORT;
    }

    HRESULT hr = E_FAIL;
    auto attr = winrt::com_ptr<ITfDisplayAttributeInfo>();
    attributes->at(index, attr.put());

    GUID guid{};
    hr = attr->GetGUID(&guid);
    CHECK_RETURN_HRESULT(hr);

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

    if (!composing()) {
        return E_ABORT;
    }

    auto hr = E_FAIL;
    auto range = winrt::com_ptr<ITfRange>();
    auto clone = winrt::com_ptr<ITfRange>();

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

} // namespace Khiin
