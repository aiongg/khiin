#pragma once

#include "KeyEvent.h"

namespace Khiin {

struct TextEngine : winrt::implements<TextEngine, IUnknown> {
    TextEngine() = default;
    TextEngine(const TextEngine &) = delete;
    TextEngine &operator=(const TextEngine &) = delete;
    ~TextEngine() = default;

    virtual void Initialize() = 0;
    virtual void Uninitialize() = 0;

    virtual void TestKey(KeyEvent keyEvent, BOOL *pConsumable) = 0;
    virtual void OnKey(KeyEvent keyEvent) = 0;
    virtual void Reset() = 0;

    virtual std::string buffer() = 0;
    virtual std::vector<std::string> &candidates() = 0;
};

struct TextEngineFactory {
    static HRESULT Create(TextEngine **ppEngine);
};

} // namespace Khiin
