#include "pch.h"

#include "CompositionUtil.h"

#include "proto/proto.h"

#include "CompositionMgr.h"
#include "Guids.h"
#include "Utils.h"

namespace khiin::win32::tip {

namespace {
using namespace winrt;
using namespace khiin::proto;

RECT ParentWindowTopLeft(ITfContextView *view) {
    RECT rect = {};
    HWND parent_hwnd = NULL;
    check_hresult(view->GetWnd(&parent_hwnd));

    if (parent_hwnd == NULL) {
        parent_hwnd = ::GetFocus();
    }

    auto found = ::GetWindowRect(parent_hwnd, &rect);

    if (found != 0) {
        return RECT{rect.left, rect.top, rect.left + 1, rect.top + 1};
    }

    return rect;
}

bool EmptyRect(RECT rect) {
    return rect.left == 0 && rect.top == 0 && rect.right == 0 && rect.bottom == 0;
}

} // namespace

com_ptr<ITfCompositionView> CompositionUtil::CompositionView(TfEditCookie cookie, ITfContext *context) {
    auto ctx = com_ptr<ITfContext>();
    ctx.copy_from(context);
    auto ctx_comp = ctx.try_as<ITfContextComposition>();

    if (!ctx_comp) {
        return nullptr;
    }

    auto enum_comp = com_ptr<IEnumITfCompositionView>();
    check_hresult(ctx_comp->FindComposition(cookie, nullptr, enum_comp.put()));

    for (;;) {
        auto view = com_ptr<ITfCompositionView>();
        ULONG fetched = 0;
        check_hresult(enum_comp->Next(1, view.put(), &fetched));
        if (fetched != 1) {
            return nullptr;
        }
        GUID clsid{};
        check_hresult(view->GetOwnerClsid(&clsid));
        if (!::IsEqualCLSID(guids::kTextService, clsid)) {
            continue;
        }
        return view;
    }
}

com_ptr<ITfRange> CompositionUtil::CompositionRange(TfEditCookie cookie, ITfContext *context) {
    auto range = com_ptr<ITfRange>();
    auto composition_view = CompositionView(cookie, context);
    check_hresult(composition_view->GetRange(range.put()));
    auto clone = com_ptr<ITfRange>();
    check_hresult(range->Clone(clone.put()));
    return clone;
}

com_ptr<ITfRange> CompositionUtil::DefaultSelectionRange(TfEditCookie cookie, ITfContext *context) {
    TF_SELECTION selection = {};
    ULONG fetched = 0;
    check_hresult(context->GetSelection(cookie, TF_DEFAULT_SELECTION, 1, &selection, &fetched));

    auto clone = com_ptr<ITfRange>();
    check_hresult(selection.range->Clone(clone.put()));
    selection.range->Release();
    return clone;
}

std::vector<InputScope> CompositionUtil::InputScopes(TfEditCookie cookie, ITfContext *context) {
    auto default_range = DefaultSelectionRange(cookie, context);
    auto prop = com_ptr<ITfReadOnlyProperty>();
    check_hresult(context->GetAppProperty(guids::kPropInputScope, prop.put()));
    VARIANT var{};
    ::VariantInit(&var);
    check_hresult(prop->GetValue(cookie, default_range.get(), &var));

    if (var.vt != VT_UNKNOWN) {
        return std::vector<InputScope>();
    }

    auto unk = com_ptr<IUnknown>();
    unk.copy_from(var.punkVal);
    auto input_scope = unk.as<ITfInputScope>();
    InputScope *buffer = nullptr;
    UINT num_scopes = 0;
    check_hresult(input_scope->GetInputScopes(&buffer, &num_scopes));
    auto ret = std::vector<InputScope>();
    ret.assign(buffer, buffer + num_scopes);
    ::CoTaskMemFree(buffer);
    return ret;
}

/**
 * Getting the position of composition text on screen with |ITfContextView::GetTextExt|
 * seems rather unreliable in some applications, so we include some fallbacks.
 *
 * 1. Try to obtain the ITfRange from the ITfCompositionView directly
 * 2. Try to obtain the ITfRange from the default selection
 *
 * In case both 1 and 2 fail, we fall back to the upper left corner of the parent
 * window, which not ideal but better than falling back to the upper left corner
 * of the entire screen.
 */
RECT CompositionUtil::TextPosition(TfEditCookie ec, ITfContext *context, int caret) {
    auto context_view = com_ptr<ITfContextView>();
    check_hresult(context->GetActiveView(context_view.put()));

    auto range = CompositionRange(ec, context);
    LONG shifted = 0;
    check_hresult(range->Collapse(ec, TF_ANCHOR_START));
    check_hresult(range->ShiftEnd(ec, caret, &shifted, nullptr));
    check_hresult(range->ShiftStart(ec, caret, &shifted, nullptr));

    RECT rect = {};
    BOOL clipped = FALSE;
    if (context_view->GetTextExt(ec, range.get(), &rect, &clipped) == S_OK && !EmptyRect(rect)) {
        KHIIN_DEBUG("From composition range {} {}", rect.left, rect.top);
        return rect;
    }

    range = nullptr;
    range = DefaultSelectionRange(ec, context);
    check_hresult(range->Collapse(ec, TF_ANCHOR_START));

    if (context_view->GetTextExt(ec, range.get(), &rect, &clipped) == S_OK && !EmptyRect(rect)) {
        KHIIN_DEBUG("From default selection {} {}", rect.left, rect.top);
        return rect;
    }

    rect = ParentWindowTopLeft(context_view.get());
    KHIIN_DEBUG("From parent window: {} {}", rect.left, rect.top);
    return rect;
}

std::wstring CompositionUtil::TextFromRange(TfEditCookie cookie, ITfRange *range) {
    auto ret = std::wstring();
    if (!range) {
        return ret;
    }

    auto copy = com_ptr<ITfRange>();
    check_hresult(range->Clone(copy.put()));
    static constexpr size_t buf_size = 1000;
    std::unique_ptr<wchar_t[]> buffer(new wchar_t[buf_size]);
    ULONG fetched = 0;
    check_hresult(copy->GetText(cookie, TF_TF_MOVESTART, buffer.get(), buf_size, &fetched));
    ret.append(buffer.get(), fetched);
    return ret;
}

WidePreedit const CompositionUtil::WidenPreedit(const Preedit &preedit) {
    auto ret = WidePreedit{};
    auto &segments = preedit.segments();
    auto start_idx = 0;

    for (auto &segment : segments) {
        auto w = Utils::Widen(segment.value());
        auto wsize = static_cast<int>(w.size());
        KHIIN_INFO(L"Segment: {}, Start: {}, Size: {}", w, start_idx, wsize);
        ret.preedit_display += w;
        ret.segment_start_and_size.push_back(std::make_pair(start_idx, wsize));
        ret.segment_status.push_back(segment.status());
        start_idx += static_cast<int>(w.size());
    }

    ret.caret = preedit.caret();
    ret.display_size = static_cast<int>(ret.preedit_display.size());
    return ret;
}

} // namespace khiin::win32::tip
