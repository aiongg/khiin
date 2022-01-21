#pragma once

#include "pch.h"

#include "KeyEvent.h"

namespace khiin::win32 {

struct TextEngine;

struct TextService : winrt::implements<TextService, IUnknown> {
    TextService() = default;
    TextService(const TextService &) = delete;
    TextService &operator=(const TextService &) = delete;
    ~TextService() = default;

    virtual TfClientId clientId() = 0;
    virtual DWORD activateFlags() = 0;

    virtual ITfThreadMgr *threadMgr() = 0;
    virtual IUnknown *compositionMgr() = 0;
    virtual IEnumTfDisplayAttributeInfo *displayAttrInfoEnum() = 0;
    virtual TextEngine *engine() = 0;
    virtual ITfUIElement *candidateUI() = 0;

    virtual winrt::com_ptr<ITfCategoryMgr> categoryMgr() = 0;
    virtual winrt::com_ptr<ITfContext> GetTopContext() = 0;
    virtual winrt::com_ptr<ITfCompositionSink> CreateCompositionSink(ITfContext *context) = 0;
    virtual void OnCompositionTerminated(TfEditCookie ecWrite, ITfContext *context, ITfComposition *pComposition) = 0;
};

struct TextServiceFactory {
    static void Create(TextService **ppService);
};

} // namespace khiin::win32
