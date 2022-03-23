#include "pch.h"

#include "CompositionMgr.h"

#include "proto/proto.h"

#include "CompositionUtil.h"
#include "DisplayAttributeInfoEnum.h"
#include "EditSession.h"
#include "TextService.h"
#include "Utils.h"
#include "common.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;
using namespace khiin::proto;

struct CompositionMgrImpl : implements<CompositionMgrImpl, CompositionMgr> {
    void Initialize(TextService *service) override {
        KHIIN_TRACE("");
        m_service.copy_from(service);
    }

    void ClearComposition(TfEditCookie cookie) override {
        if (m_composition) {
            m_composition->EndComposition(cookie);
        }
        m_composition = nullptr;
    }

    void Uninitialize() override {
        KHIIN_TRACE("");
        m_service = nullptr;
        m_composition = nullptr;
    }

    bool composing() override {
        return bool(m_composition);
    }

    void DoComposition(TfEditCookie cookie, ITfContext *context, Preedit preedit) override {
        KHIIN_TRACE("");

        auto w_preedit = Utils::WidenPreedit(preedit);

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
            comp_range->SetText(cookie, TF_ST_CORRECTION, w_preedit.preedit_display.c_str(), w_preedit.display_size));

        // here we do each segment's attributes, but for now just one
        auto display_attribute = com_ptr<ITfProperty>();
        check_hresult(context->GetProperty(GUID_PROP_ATTRIBUTE, display_attribute.put()));
        auto n_segments = w_preedit.segment_start_and_size.size();
        for (size_t i = 0; i < n_segments; ++i) {
            auto &status = w_preedit.segment_status[i];
            TfGuidAtom attribute_atom = TF_INVALID_GUIDATOM;

            if (status == SS_COMPOSING) {
                attribute_atom = m_service->input_attribute();
            } else if (status == SS_CONVERTED) {
                attribute_atom = m_service->converted_attribute();
            } else if (status == SS_FOCUSED) {
                attribute_atom = m_service->focused_attribute();
            } else {
                continue;
            }

            auto &[segment_start, size] = w_preedit.segment_start_and_size[i];
            auto segment_end = segment_start + size;
            auto segment_range = com_ptr<ITfRange>();
            check_hresult(comp_range->Clone(segment_range.put()));
            check_hresult(segment_range->Collapse(cookie, TF_ANCHOR_START));
            LONG shifted = 0;
            check_hresult(segment_range->ShiftEnd(cookie, segment_end, &shifted, nullptr));
            check_hresult(segment_range->ShiftStart(cookie, segment_start, &shifted, nullptr));
            VARIANT var;
            ::VariantInit(&var);
            var.vt = VT_I4;
            var.lVal = attribute_atom;
            check_hresult(display_attribute->SetValue(cookie, segment_range.get(), &var));
        }

        // update cursor
        auto cursor_range = com_ptr<ITfRange>();
        check_hresult(comp_range->Clone(cursor_range.put()));
        auto cursor_pos = w_preedit.cursor;
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
        m_composition->EndComposition(cookie);
        m_composition = nullptr;
    }

    void CommitComposition(TfEditCookie cookie, ITfContext *context, Preedit preedit) override try {
        KHIIN_TRACE("");

        if (!m_composition) {
            StartComposition(cookie, context);
        }
        if (!m_composition) {
            return;
        }

        auto w_preedit = Utils::WidenPreedit(preedit);
        auto comp_range = com_ptr<ITfRange>();
        check_hresult(m_composition->GetRange(comp_range.put()));
        // Clone the range and move to the end
        auto end_range = com_ptr<ITfRange>();
        check_hresult(comp_range->Clone(end_range.put()));
        LONG shifted = 0;
        // Move START anchor to the end (outside of range)
        check_hresult(end_range->ShiftStart(cookie, w_preedit.display_size, &shifted, nullptr));
        // Collapse to START anchor
        check_hresult(end_range->Collapse(cookie, TF_ANCHOR_START));
        // Update composition by moving START anchor to the same position as the clone
        check_hresult(m_composition->ShiftStart(cookie, end_range.get()));

        SetSelection(cookie, context, end_range.get(), TF_AE_END);
        m_composition->EndComposition(cookie);
        m_composition = nullptr;
    } catch (...) {
        // Composition was ended
        return;
    }

    void CancelComposition(TfEditCookie cookie) override {
        KHIIN_TRACE("");
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

  private:
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

    void ApplyDisplayAttribute(TfEditCookie cookie, ITfContext *context, ITfRange *range, AttrInfoKey index) {
        KHIIN_TRACE("");
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
