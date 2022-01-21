#pragma once

#include "DisplayAttributeInfo.h"
#include "DisplayAttributeInfoEnum.h"

#include "TextService.h"

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

    void StartComposition(TfEditCookie cookie, ITfContext *pContext);
    void DoComposition(TfEditCookie cookie, ITfContext *pContext, std::string text);
    void EndComposition(TfEditCookie cookie);
    void GetTextRange(TfEditCookie cookie, ITfRange **ppRange);

  private:
    void ApplyDisplayAttribute(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange, AttrInfoKey index);
    void CollapseCursorToEnd(TfEditCookie cookie, ITfContext *pContext);

    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<DisplayAttributeInfoEnum> attributes = nullptr;
    winrt::com_ptr<ITfComposition> composition = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    winrt::com_ptr<ITfCategoryMgr> categoryMgr = nullptr;
};

} // namespace khiin::win32
