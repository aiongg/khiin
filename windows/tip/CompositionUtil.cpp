#include "pch.h"

#include "CompositionUtil.h"

#include "CompositionMgr.h"
#include "Guids.h"

namespace khiin::win32::tip {

namespace {
using namespace winrt;

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

HRESULT TextExtFromRange(TfEditCookie cookie, ITfContextView *view, ITfRange *range, RECT &rc) {
    range->Collapse(cookie, TF_ANCHOR_START);
    RECT rect{};
    BOOL clipped = FALSE;
    auto hr = view->GetTextExt(cookie, range, &rect, &clipped);
    rc = rect;
    return hr;
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

com_ptr<ITfRange> CompositionUtil::DefaultSelectionRange(TfEditCookie cookie, ITfContext *context) {
    TF_SELECTION selection = {};
    ULONG fetched = 0;
    check_hresult(context->GetSelection(cookie, TF_DEFAULT_SELECTION, 1, &selection, &fetched));

    auto range = com_ptr<ITfRange>();
    check_hresult(selection.range->Clone(range.put()));
    selection.range->Release();
    return range;
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

RECT CompositionUtil::TextPosition(TfEditCookie ec, ITfContext *context) {
    RECT rect = {};
    HRESULT hr = E_FAIL;
    auto range = com_ptr<ITfRange>();
    auto composition_view = CompositionView(ec, context);
    composition_view->GetRange(range.put());

    auto context_view = com_ptr<ITfContextView>();
    context->GetActiveView(context_view.put());
    hr = TextExtFromRange(ec, context_view.get(), range.get(), rect);
    KHIIN_DEBUG("From composition range {} {} {} {}", rect.left, rect.top, rect.right, rect.bottom);
    CHECK_HRESULT(hr);
    if (hr == S_OK) {
        return rect;
    }

    return ParentWindowTopLeft(context_view.get());
}

} // namespace khiin::win32::tip
