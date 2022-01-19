#pragma once

#include "DisplayAttributeInfo.h"
#include "DisplayAttributeInfoEnum.h"

#include "TextService.h"

namespace Khiin {

struct CompositionMgr : winrt::implements<CompositionMgr, IUnknown> {
    CompositionMgr() = default;
    ~CompositionMgr();

    HRESULT init(TextService *pTextService);
    void clearComposition();
    HRESULT uninit();

    bool composing();

    HRESULT startComposition(TfEditCookie cookie, ITfContext *pContext);
    HRESULT doComposition(TfEditCookie cookie, ITfContext *pContext, std::string text);
    HRESULT endComposition(TfEditCookie cookie);
    HRESULT GetTextRange(TfEditCookie cookie, ITfRange **ppRange);

  private:
    HRESULT applyDisplayAttribute(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange, AttrInfoKey index);
    HRESULT collapseCursorToEnd(TfEditCookie cookie, ITfContext *pContext);

    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<DisplayAttributeInfoEnum> attributes = nullptr;
    winrt::com_ptr<ITfComposition> composition = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    winrt::com_ptr<ITfCategoryMgr> categoryMgr = nullptr;

    DELETE_COPY_AND_ASSIGN(CompositionMgr);
};

} // namespace Khiin
