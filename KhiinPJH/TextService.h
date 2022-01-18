#pragma once

#include "pch.h"

#include "KeyEvent.h"

namespace Khiin {

struct TextEngine;

struct TextService : winrt::implements<TextService, IUnknown> {
    virtual TfClientId clientId() = 0;
    virtual DWORD activateFlags() = 0;

    virtual ITfThreadMgr *threadMgr() = 0;
    virtual IUnknown *compositionMgr() = 0;
    virtual IEnumTfDisplayAttributeInfo *displayAttrInfoEnum() = 0;
    virtual TextEngine *engine() = 0;
    virtual ITfUIElement *candidateUI() = 0;

    virtual HRESULT topContext(ITfContext **ppContext) = 0;
    virtual HRESULT categoryMgr(ITfCategoryMgr **ppCategoryMgr) = 0;
    virtual HRESULT compositionSink(ITfContext *context, ITfCompositionSink **ppCompositionSink) = 0;
    virtual HRESULT onCompositionTerminated(TfEditCookie ecWrite, ITfContext *context, ITfComposition *pComposition) = 0;

    //virtual HRESULT consumeKey(ITfContext *pContext, KeyEvent keyEvent) = 0;
    //virtual HRESULT updateContext(ITfContext *pContext, TfEditCookie writeEc, KeyEvent keyEvent) = 0;

    // virtual HRESULT beginEditSession();

    DEFAULT_CTOR_DTOR(TextService);
    DELETE_COPY_AND_ASSIGN(TextService);
};

struct TextServiceFactory {
    static HRESULT create(TextService **ppService);
};

} // namespace Khiin
