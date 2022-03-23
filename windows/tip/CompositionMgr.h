#pragma once

#include "DisplayAttributeInfo.h"
#include "DisplayAttributeInfoEnum.h"

namespace khiin::win32::tip {

struct TextService;
struct WidePreedit;

struct CompositionMgr : winrt::implements<CompositionMgr, IUnknown> {
    CompositionMgr() = default;
    CompositionMgr(const CompositionMgr &) = delete;
    CompositionMgr &operator=(const CompositionMgr &) = delete;
    ~CompositionMgr() = default;

    static winrt::com_ptr<CompositionMgr> Create();
    virtual void Initialize(TextService *service) = 0;
    virtual void Uninitialize() = 0;

    virtual bool composing() = 0;

    virtual void DoComposition(TfEditCookie cookie, ITfContext *context, WidePreedit comp_data) = 0;
    virtual void CommitComposition(TfEditCookie cookie, ITfContext *context) = 0;
    virtual void CommitComposition(TfEditCookie cookie, ITfContext *context, WidePreedit comp_data) = 0;
    virtual void CancelComposition(TfEditCookie cookie) = 0;
    virtual void ClearComposition(TfEditCookie cookie) = 0;

    virtual winrt::com_ptr<ITfRange> GetTextRange(TfEditCookie cookie) = 0;
};

} // namespace khiin::win32::tip
