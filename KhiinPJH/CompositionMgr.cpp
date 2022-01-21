#include "pch.h"

#include "CompositionMgr.h"

#include "DisplayAttributeInfoEnum.h"
#include "EditSession.h"

#include "common.h"

namespace khiin::win32 {

//+---------------------------------------------------------------------------
//
// Public methods
//
//----------------------------------------------------------------------------

CompositionMgr::~CompositionMgr() {
    Uninitialize();
}

void CompositionMgr::Initialize(TextService *pTextService) {
    D(__FUNCTIONW__);
    service.copy_from(pTextService);
    attributes.copy_from(cast_as<DisplayAttributeInfoEnum>(service->displayAttrInfoEnum()));
    WINRT_ASSERT(attributes);
    categoryMgr = service->categoryMgr();
}

void CompositionMgr::ClearComposition() {
    composition = nullptr;
}

void CompositionMgr::Uninitialize() {
    D(__FUNCTIONW__);
    service = nullptr;
    attributes = nullptr;
    categoryMgr = nullptr;
    composition = nullptr;
    context = nullptr;
}

bool CompositionMgr::composing() {
    return bool(composition);
}

//+---------------------------------------------------------------------------
//
// Private methods
//
//----------------------------------------------------------------------------

void CompositionMgr::StartComposition(TfEditCookie cookie, ITfContext *pContext) {
    D(__FUNCTIONW__);

    auto context = winrt::com_ptr<ITfContext>();
    context.copy_from(pContext);

    auto insertAtSelection = context.as<ITfInsertAtSelection>();
    auto contextComposition = context.as<ITfContextComposition>();

    auto range = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(insertAtSelection->InsertTextAtSelection(cookie, TF_IAS_QUERYONLY, NULL, 0, range.put()));

    auto composition = winrt::com_ptr<ITfComposition>();
    auto compositionSink = service->CreateCompositionSink(pContext);
    winrt::check_hresult(contextComposition->StartComposition(cookie, range.get(), compositionSink.get(), composition.put()));

    this->composition = nullptr;
    this->composition = composition;

    TF_SELECTION sel{};
    sel.range = range.get();
    sel.style.ase = TF_AE_NONE;
    sel.style.fInterimChar = FALSE;
    winrt::check_hresult(context->SetSelection(cookie, 1, &sel));

    this->context = nullptr;
    this->context = context;
}

void CompositionMgr::DoComposition(TfEditCookie cookie, ITfContext *pContext, std::string text) {
    D(__FUNCTIONW__);

    if (!composing()) {
        StartComposition(cookie, pContext);
    }

    auto range = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(composition->GetRange(range.put()));

    auto wstr = std::wstring(text.cbegin(), text.cend());
    auto wstrlen = static_cast<LONG>(wstr.size());
    winrt::check_hresult( range->SetText(cookie, TF_ST_CORRECTION, &wstr[0], wstrlen));

    ApplyDisplayAttribute(cookie, pContext, range.get(), AttrInfoKey::Input);
    CollapseCursorToEnd(cookie, pContext);
}

void CompositionMgr::EndComposition(TfEditCookie cookie) {
    D(__FUNCTIONW__);

    if (!composing()) {
        return;
    }

    CollapseCursorToEnd(cookie, context.get());
    winrt::check_hresult(composition->EndComposition(cookie));
    composition = nullptr;
}

void CompositionMgr::GetTextRange(TfEditCookie cookie, ITfRange **ppRange) {
    if (!composing()) {
        return;
    }

    auto range = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(composition->GetRange(range.put()));
    range.copy_to(ppRange);
}

void CompositionMgr::ApplyDisplayAttribute(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange,
                                              AttrInfoKey index) {
    D(__FUNCTIONW__);

    if (!composing()) {
        return;
    }

    auto attr = winrt::com_ptr<ITfDisplayAttributeInfo>();
    attributes->at(index, attr.put());

    GUID guid{};
    winrt::check_hresult(attr->GetGUID(&guid));

    TfGuidAtom atom = TF_INVALID_GUIDATOM;
    winrt::check_hresult(categoryMgr->RegisterGUID(guid, &atom));

    if (atom == TF_INVALID_GUIDATOM) {
        throw winrt::hresult_invalid_argument();
    }

    auto prop = winrt::com_ptr<ITfProperty>();
    winrt::check_hresult(pContext->GetProperty(GUID_PROP_ATTRIBUTE, prop.put()));

    VARIANT var;
    ::VariantInit(&var);
    var.vt = VT_I4;
    var.lVal = atom;

    winrt::check_hresult(prop->SetValue(cookie, pRange, &var));
}

void CompositionMgr::CollapseCursorToEnd(TfEditCookie cookie, ITfContext *pContext) {
    D(__FUNCTIONW__);

    if (!composing()) {
        return;
    }

    auto range = winrt::com_ptr<ITfRange>();
    auto clone = winrt::com_ptr<ITfRange>();

    winrt::check_hresult(composition->GetRange(range.put()));
    winrt::check_hresult(range->Clone(clone.put()));
    winrt::check_hresult(clone->Collapse(cookie, TF_ANCHOR_END));

    TF_SELECTION sel{};
    sel.range = clone.get();
    sel.style.ase = TF_AE_NONE;
    sel.style.fInterimChar = FALSE;
    winrt::check_hresult(pContext->SetSelection(cookie, 1, &sel));
}

} // namespace khiin::win32
