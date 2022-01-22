#pragma once

#include "Action.h"
#include "KeyEvent.h"

namespace khiin::win32 {

struct TextEngine : winrt::implements<TextEngine, IUnknown> {
    TextEngine() = default;
    TextEngine(const TextEngine &) = delete;
    TextEngine &operator=(const TextEngine &) = delete;
    ~TextEngine() = default;

    virtual void Initialize() = 0;
    virtual void Uninitialize() = 0;

    virtual Action TestKey(KeyEvent keyEvent) = 0;
    virtual Action OnKey(KeyEvent keyEvent) = 0;
    virtual void Reset() = 0;
};

struct TextEngineFactory {
    static void OnDllProcessAttach(HMODULE module);
    static void OnDllProcessDetach(HMODULE module);
    static HRESULT Create(TextEngine **ppEngine);

};

} // namespace khiin::win32
