#pragma once

#include "DisplayAttributeInfo.h"
#include "DisplayAttributeInfoEnum.h"

namespace Khiin {

struct CompositionMgr : winrt::implements<CompositionMgr, ITfCompositionSink> {

    CompositionMgr() = default;
    CompositionMgr(const CompositionMgr &) = delete;
    CompositionMgr &operator=(const CompositionMgr &) = delete;
    ~CompositionMgr() = default;

    HRESULT init(TfClientId clientId, DisplayAttributeInfoEnum *pDaiEnum);
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
    HRESULT applyDisplayAttribute(TfEditCookie cookie, ITfContext *pContext, ITfRange *pRange, AttributeIndex index);
    HRESULT collapseCursorToEnd(TfEditCookie cookie, ITfContext *pContext);

    TfClientId clientId = TF_CLIENTID_NULL;
    winrt::com_ptr<DisplayAttributeInfoEnum> attributes = nullptr;
    winrt::com_ptr<ITfComposition> composition = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    winrt::com_ptr<ITfCategoryMgr> categoryMgr = nullptr;
};

} // namespace Khiin
