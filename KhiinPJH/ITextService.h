#pragma once

#include "pch.h"

namespace Khiin {

struct ITextEngine;

struct ITextService : winrt::implements<ITextService, IUnknown> {
    virtual TfClientId clientId() = 0;
    virtual ITfThreadMgr *threadMgr() = 0;
    virtual DWORD activateFlags() = 0;
    virtual ITfCompositionSink *compositionMgr() = 0;
    virtual IEnumTfDisplayAttributeInfo *displayAttrInfoEnum() = 0;
    virtual HRESULT topContext(ITfContext **ppContext) = 0;
    virtual ITextEngine *engine() = 0;
    virtual ITfUIElement *candidateUI() = 0;

    // virtual HRESULT beginEditSession();

    DEFAULT_CTOR_DTOR(ITextService);
    DELETE_COPY_AND_ASSIGN(ITextService);
};

struct TextServiceFactory {
    static HRESULT create(ITextService **ppService);
};

} // namespace Khiin
