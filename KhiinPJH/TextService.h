#pragma once

#include "pch.h"

namespace Khiin {

struct TextService : winrt::implements<TextService, IUnknown> {
    virtual TfClientId clientId();
    virtual ITfThreadMgr *threadMgr();
    virtual ITfCompositionSink *compositionMgr();
    virtual IEnumTfDisplayAttributeInfo *displayAttrInfoEnum();
    virtual DWORD getActivateFlags();

    DEFAULT_CTOR_DTOR(TextService);
    DELETE_COPY_AND_ASSIGN(TextService);
};

struct TextServiceFactory {
    static TextService *create();
};

} // namespace Khiin
