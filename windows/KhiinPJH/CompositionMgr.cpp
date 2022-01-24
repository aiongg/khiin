#include "pch.h"

#include "CompositionMgr.h"

#include "DisplayAttributeInfoEnum.h"
#include "EditSession.h"
#include "Utils.h"
#include "common.h"

namespace khiin::win32 {

using namespace messages;

inline static int kMaxBufSize = 512;

std::string CompDataToString(Composition comp_data) {
    auto ret = std::string();
    auto &segments = comp_data.segments();
    for (auto &pSeg : segments) {
        ret += pSeg.value();
    }
    return ret;
}

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
}

void CompositionMgr::ClearComposition() {
    composition = nullptr;
}

void CompositionMgr::Uninitialize() {
    D(__FUNCTIONW__);
    service = nullptr;
    composition = nullptr;
}

bool CompositionMgr::composing() {
    return bool(composition);
}

void CompositionMgr::DoComposition(TfEditCookie cookie, ITfContext *pContext, Composition comp_data) {
    D(__FUNCTIONW__);

    auto display_text = CompDataToString(comp_data);

    if (composition) {
        // clear existing composition
        auto range = winrt::com_ptr<ITfRange>();
        winrt::check_hresult(composition->GetRange(range.put()));

        auto is_empty = FALSE;
        winrt::check_hresult(range->IsEmpty(cookie, &is_empty));
        if (is_empty != TRUE) {
            winrt::check_hresult(range->SetText(cookie, 0, L"", 0));
        }
    } else {
        // or get a new one
        StartComposition(cookie, pContext);
        WINRT_ASSERT(composition);
    }

    // set text
    auto comp_range = winrt::com_ptr<ITfRange>();
    auto wdisplay_text = Utils::Widen(display_text);
    winrt::check_hresult(composition->GetRange(comp_range.put()));
    winrt::check_hresult(comp_range->SetText(cookie, 0, wdisplay_text.c_str(), wdisplay_text.size()));

    // here we do each segment's attributes, but for now just one
    auto display_attribute = winrt::com_ptr<ITfProperty>();
    winrt::check_hresult(pContext->GetProperty(GUID_PROP_ATTRIBUTE, display_attribute.put()));
    {
        auto attribute_atom = service->input_attribute();
        auto segment_start = 0;
        auto segment_end = wdisplay_text.size();
        auto segment_range = winrt::com_ptr<ITfRange>();
        winrt::check_hresult(comp_range->Clone(segment_range.put()));
        winrt::check_hresult(segment_range->Collapse(cookie, TF_ANCHOR_START));
        LONG shifted = 0;
        winrt::check_hresult(segment_range->ShiftEnd(cookie, segment_end, &shifted, nullptr));
        winrt::check_hresult(segment_range->ShiftStart(cookie, segment_start, &shifted, nullptr));
        VARIANT var;
        ::VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = attribute_atom;
        winrt::check_hresult(display_attribute->SetValue(cookie, segment_range.get(), &var));
    }

    // update cursor
    auto cursor_range = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(comp_range->Clone(cursor_range.put()));
    auto cursor_pos = wdisplay_text.size();
    winrt::check_hresult(cursor_range->Collapse(cookie, TF_ANCHOR_START));
    LONG shifted = 0;
    winrt::check_hresult(cursor_range->ShiftEnd(cookie, cursor_pos, &shifted, nullptr));
    winrt::check_hresult(cursor_range->ShiftStart(cookie, cursor_pos, &shifted, nullptr));

    SetSelection(cookie, pContext, cursor_range.get(), TF_AE_END);
}

void CompositionMgr::CommitComposition(TfEditCookie cookie, ITfContext* pContext) {
    if (!composition) {
        return;
    }
    auto comp_range = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(composition->GetRange(comp_range.put()));
    
    auto end_range = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(comp_range->Clone(end_range.put()));
    winrt::check_hresult(end_range->Collapse(cookie, TF_ANCHOR_END));
    winrt::check_hresult(composition->ShiftStart(cookie, end_range.get()));
    SetSelection(cookie, pContext, end_range.get(), TF_AE_END);
    composition = nullptr;
}

void CompositionMgr::CommitComposition(TfEditCookie cookie, ITfContext *pContext, Composition comp_data) try {
    D(__FUNCTIONW__);

    if (!composition) {
        StartComposition(cookie, pContext);
    }
    if (!composition) {
        return;
    }

    auto commit_text = CompDataToString(comp_data);
    auto wcommit_text = Utils::Widen(commit_text);
    auto comp_range = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(composition->GetRange(comp_range.put()));
    // Clone the range and move to the end
    auto end_range = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(comp_range->Clone(end_range.put()));
    LONG shifted = 0;
    // Move START anchor to the end (outside of range)
    winrt::check_hresult(end_range->ShiftStart(cookie, wcommit_text.size(), &shifted, nullptr));
    // Collapse to START anchor
    winrt::check_hresult(end_range->Collapse(cookie, TF_ANCHOR_START));
    // Update composition by moving START anchor to the same position as the clone
    winrt::check_hresult(composition->ShiftStart(cookie, end_range.get()));

    SetSelection(cookie, pContext, end_range.get(), TF_AE_END);

    composition = nullptr;
} catch (...) {
    // Composition was ended
    return;
}

void CompositionMgr::CancelComposition(TfEditCookie cookie) {}

void CompositionMgr::GetTextRange(TfEditCookie cookie, ITfRange **ppRange) {
    if (!composing()) {
        return;
    }

    auto range = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(composition->GetRange(range.put()));
    range.copy_to(ppRange);
}

//+---------------------------------------------------------------------------
//
// Private methods
//
//----------------------------------------------------------------------------

void CompositionMgr::StartComposition(TfEditCookie cookie, ITfContext *pContext) {
    auto context = winrt::com_ptr<ITfContext>();
    context.copy_from(pContext);
    auto insert_selection = context.as<ITfInsertAtSelection>();
    auto insert_position = winrt::com_ptr<ITfRange>();
    winrt::check_hresult(
        insert_selection->InsertTextAtSelection(cookie, TF_IAS_QUERYONLY, nullptr, 0, insert_position.put()));
    auto composition_context = context.as<ITfContextComposition>();
    winrt::check_hresult(composition_context->StartComposition(
        cookie, insert_position.get(), service->CreateCompositionSink(context.get()).get(), composition.put()));
}

void CompositionMgr::ApplyDisplayAttribute(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange,
                                           AttrInfoKey index) {}

void CompositionMgr::SetSelection(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange, TfActiveSelEnd ase) {
    TF_SELECTION sel{};
    sel.range = pRange;
    sel.style.ase = ase;
    sel.style.fInterimChar = FALSE;
    winrt::check_hresult(pContext->SetSelection(cookie, 1, &sel));
}

} // namespace khiin::win32
