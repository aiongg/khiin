#pragma once

#include "DisplayAttributeInfo.h"
#include "DisplayAttributeInfoEnum.h"
#include "TextService.h"
#include "common.h"

namespace khiin::win32 {

struct CompositionMgr : winrt::implements<CompositionMgr, IUnknown> {
    CompositionMgr() = default;
    CompositionMgr(const CompositionMgr &) = delete;
    CompositionMgr &operator=(const CompositionMgr &) = delete;
    ~CompositionMgr();

    void Initialize(TextService *pTextService);
    void Uninitialize();

    void ClearComposition();

    bool composing();

    void DoComposition(TfEditCookie cookie, ITfContext *pContext, proto::Preedit comp_data);
    void CommitComposition(TfEditCookie cookie, ITfContext *pContext);
    void CommitComposition(TfEditCookie cookie, ITfContext *pContext, proto::Preedit comp_data);
    void CancelComposition(TfEditCookie cookie);
    void GetTextRange(TfEditCookie cookie, ITfRange **ppRange);

  private:
    void StartComposition(TfEditCookie cookie, ITfContext *pContext);
    void ApplyDisplayAttribute(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange, AttrInfoKey index);
    //void CollapseCursorToEnd(TfEditCookie cookie, ITfContext *pContext);
    void SetSelection(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange, TfActiveSelEnd active_sel_end);

    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfComposition> composition = nullptr;
    proto::Preedit composition_data;
};

} // namespace khiin::win32
