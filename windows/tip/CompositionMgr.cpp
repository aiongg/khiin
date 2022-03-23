#include "pch.h"

#include "CompositionMgr.h"

#include "CompositionUtil.h"
#include "DisplayAttributeInfoEnum.h"
#include "EditSession.h"
#include "TextService.h"
#include "common.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;

struct CompositionMgrImpl : implements<CompositionMgrImpl, CompositionMgr> {
  private:
    void Initialize(TextService *service) override {
        KHIIN_TRACE("");
        m_service.copy_from(service);
    }

    void ClearComposition(TfEditCookie cookie) override {
        Cleanup(cookie);
    }

    void Uninitialize() override {
        KHIIN_TRACE("");
        m_service = nullptr;
        m_composition = nullptr;
    }

    bool composing() override {
        return bool(m_composition);
    }

    void DoComposition(TfEditCookie cookie, ITfContext *context, WidePreedit preedit) override {
        KHIIN_TRACE("");

        if (m_composition) {
            // clear existing composition
            auto range = com_ptr<ITfRange>();
            check_hresult(m_composition->GetRange(range.put()));

            auto is_empty = FALSE;
            check_hresult(range->IsEmpty(cookie, &is_empty));
            if (is_empty == FALSE) {
                check_hresult(range->SetText(cookie, TF_ST_CORRECTION, L"", 0));
            }
        } else {
            // or get a new one
            StartComposition(cookie, context);
            WINRT_ASSERT(m_composition);
        }

        // set text
        auto comp_range = com_ptr<ITfRange>();
        check_hresult(m_composition->GetRange(comp_range.put()));
        check_hresult(
            comp_range->SetText(cookie, TF_ST_CORRECTION, preedit.preedit_display.c_str(), preedit.display_size));

        // here we do each segment's attributes, but for now just one
        auto display_attribute = com_ptr<ITfProperty>();
        check_hresult(context->GetProperty(GUID_PROP_ATTRIBUTE, display_attribute.put()));
        auto n_segments = preedit.segment_start_and_size.size();
        for (size_t i = 0; i < n_segments; ++i) {
            auto status = preedit.segment_status[i];
            auto &[start, size] = preedit.segment_start_and_size[i];
            ApplyDisplayAttribute(cookie, display_attribute.get(), comp_range.get(), start, size, status);
        }

        // update caret
        auto cursor_range = com_ptr<ITfRange>();
        check_hresult(comp_range->Clone(cursor_range.put()));
        auto cursor_pos = preedit.caret;
        check_hresult(cursor_range->Collapse(cookie, TF_ANCHOR_START));
        LONG shifted = 0;
        check_hresult(cursor_range->ShiftEnd(cookie, cursor_pos, &shifted, nullptr));
        check_hresult(cursor_range->ShiftStart(cookie, cursor_pos, &shifted, nullptr));

        SetSelection(cookie, context, cursor_range.get(), TF_AE_END);
    }

    void CommitComposition(TfEditCookie cookie, ITfContext *context) override {
        KHIIN_TRACE("");
        if (!m_composition) {
            return;
        }
        auto comp_range = com_ptr<ITfRange>();
        check_hresult(m_composition->GetRange(comp_range.put()));
        auto text = CompositionUtil::TextFromRange(cookie, comp_range.get());

        auto display_attribute = com_ptr<ITfProperty>();
        check_hresult(context->GetProperty(GUID_PROP_ATTRIBUTE, display_attribute.put()));
        check_hresult(display_attribute->Clear(cookie, comp_range.get()));

        auto next_range = com_ptr<ITfRange>();
        check_hresult(comp_range->Clone(next_range.put()));

        LONG shifted = 0;
        check_hresult(next_range->ShiftStart(cookie, static_cast<LONG>(text.size()), &shifted, nullptr));
        check_hresult(next_range->Collapse(cookie, TF_ANCHOR_START));
        check_hresult(m_composition->ShiftStart(cookie, next_range.get()));
        SetSelection(cookie, context, next_range.get(), TF_AE_END);
        Cleanup(cookie);
    }

    void CommitComposition(TfEditCookie cookie, ITfContext *context, WidePreedit preedit) override try {
        KHIIN_TRACE("");

        if (!m_composition) {
            StartComposition(cookie, context);
        }
        if (!m_composition) {
            return;
        }

        auto comp_range = com_ptr<ITfRange>();
        check_hresult(m_composition->GetRange(comp_range.put()));
        // Clone the composition_range and move to the end
        auto end_range = com_ptr<ITfRange>();
        check_hresult(comp_range->Clone(end_range.put()));
        LONG shifted = 0;
        // Move START anchor to the end (outside of composition_range)
        check_hresult(end_range->ShiftStart(cookie, preedit.display_size, &shifted, nullptr));
        // Collapse to START anchor
        check_hresult(end_range->Collapse(cookie, TF_ANCHOR_START));
        // Update composition by moving START anchor to the same position as the clone
        check_hresult(m_composition->ShiftStart(cookie, end_range.get()));

        SetSelection(cookie, context, end_range.get(), TF_AE_END);
        Cleanup(cookie);
    } catch (...) {
        // Composition was ended
        return;
    }

    void CancelComposition(TfEditCookie cookie) override {
        KHIIN_TRACE("");
        Cleanup(cookie);
    }

    void Cleanup(TfEditCookie cookie) {
        if (m_composition) {
            m_composition->EndComposition(cookie);
            m_composition = nullptr;
        }
    }

    com_ptr<ITfRange> GetTextRange(TfEditCookie cookie) override {
        KHIIN_TRACE("");
        if (!composing()) {
            return nullptr;
        }

        auto range = com_ptr<ITfRange>();
        check_hresult(m_composition->GetRange(range.put()));
        auto clone = com_ptr<ITfRange>();
        check_hresult(range->Clone(clone.put()));
        return clone;
    }

    void StartComposition(TfEditCookie cookie, ITfContext *context) {
        KHIIN_TRACE("");
        auto ctx = com_ptr<ITfContext>();
        ctx.copy_from(context);
        auto insert_selection = ctx.as<ITfInsertAtSelection>();
        auto insert_position = com_ptr<ITfRange>();
        check_hresult(
            insert_selection->InsertTextAtSelection(cookie, TF_IAS_QUERYONLY, nullptr, 0, insert_position.put()));
        auto composition_context = ctx.as<ITfContextComposition>();
        check_hresult(composition_context->StartComposition(
            cookie, insert_position.get(), m_service->CreateCompositionSink(context).get(), m_composition.put()));
        SetSelection(cookie, context, insert_position.get(), TF_AE_NONE);
    }

    void ApplyDisplayAttribute(TfEditCookie cookie, ITfProperty *attribute_property, ITfRange *composition_range,
                               int start, int size, SegmentStatus status) {
        KHIIN_TRACE("");
        auto attribute_atom = m_service->DisplayAttributeAtom(status);

        if (attribute_atom == TF_INVALID_GUIDATOM) {
            return;
        }

        auto end = start + size;
        auto segment_range = com_ptr<ITfRange>();
        check_hresult(composition_range->Clone(segment_range.put()));
        check_hresult(segment_range->Collapse(cookie, TF_ANCHOR_START));
        LONG shifted = 0;
        check_hresult(segment_range->ShiftEnd(cookie, end, &shifted, nullptr));
        check_hresult(segment_range->ShiftStart(cookie, start, &shifted, nullptr));
        VARIANT var;
        ::VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = attribute_atom;
        check_hresult(attribute_property->SetValue(cookie, segment_range.get(), &var));
    }

    void SetSelection(TfEditCookie cookie, ITfContext *context, ITfRange *range, TfActiveSelEnd ase) {
        KHIIN_TRACE("");
        TF_SELECTION sel{};
        sel.range = range;
        sel.style.ase = ase;
        sel.style.fInterimChar = FALSE;
        check_hresult(context->SetSelection(cookie, 1, &sel));
    }

    com_ptr<TextService> m_service = nullptr;
    com_ptr<ITfComposition> m_composition = nullptr;
};

} // namespace

//+---------------------------------------------------------------------------
//
// Public methods
//
//----------------------------------------------------------------------------

com_ptr<CompositionMgr> CompositionMgr::Create() {
    return as_self<CompositionMgr>(make_self<CompositionMgrImpl>());
}

} // namespace khiin::win32::tip
