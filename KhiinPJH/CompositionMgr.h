#pragma once

#include "DisplayAttributeInfo.h"
#include "DisplayAttributeInfoEnum.h"

#include "ITextService.h"

namespace Khiin {

struct CompositionMgr : winrt::implements<CompositionMgr, ITfCompositionSink> {
    CompositionMgr() = default;
    ~CompositionMgr();
    DELETE_COPY_AND_ASSIGN(CompositionMgr);

    HRESULT init(ITextService *pTextService);
    HRESULT uninit();

    bool composing();

    HRESULT startComposition(ITfContext *pContext);
    HRESULT doComposition(ITfContext *pContext, std::string text);
    HRESULT endComposition();

    // ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(TfEditCookie cookie, ITfComposition *pComposition) override;

  private:
    HRESULT startComposition(TfEditCookie cookie, ITfContext *pContext);
    HRESULT doComposition(TfEditCookie cookie, ITfContext *pContext, std::string text);
    HRESULT endComposition(TfEditCookie cookie);

    HRESULT setText(TfEditCookie cookie, std::string_view text);
    HRESULT applyDisplayAttribute(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange, AttrInfoKey index);
    HRESULT collapseCursorToEnd(TfEditCookie cookie, ITfContext *pContext);

    winrt::com_ptr<ITextService> textService = nullptr;
    winrt::com_ptr<DisplayAttributeInfoEnum> attributes = nullptr;
    winrt::com_ptr<ITfComposition> composition = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    winrt::com_ptr<ITfCategoryMgr> categoryMgr = nullptr;
};

} // namespace Khiin
