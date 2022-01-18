#pragma once

#include "KeyEvent.h"

namespace Khiin {

struct TextEngine : winrt::implements<TextEngine, IUnknown> {
    virtual HRESULT init() = 0;
    virtual HRESULT uninit() = 0;

    virtual HRESULT onTestKey(KeyEvent keyEvent, BOOL *pConsumable) = 0;
    virtual HRESULT onKey(KeyEvent keyEvent) = 0;
    virtual HRESULT clear() = 0;

    virtual std::string buffer() = 0;
    virtual std::vector<std::string> &candidates() = 0;

    DEFAULT_CTOR_DTOR(TextEngine);
    DELETE_COPY_AND_ASSIGN(TextEngine);
};

struct TextEngineFactory {
    static HRESULT create(TextEngine **ppEngine);
};

} // namespace Khiin
