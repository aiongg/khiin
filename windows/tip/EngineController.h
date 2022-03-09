#pragma once

#include "KeyEvent.h"

namespace khiin::proto {
class Command;
}

namespace khiin::win32::tip {

struct TextService;

struct EngineController : winrt::implements<EngineController, IUnknown> {
    EngineController() = default;
    EngineController(const EngineController &) = delete;
    EngineController &operator=(const EngineController &) = delete;
    ~EngineController() = default;

    virtual void Initialize(TextService *pService) = 0;
    virtual void Uninitialize() = 0;

    virtual proto::Command *TestKey(win32::KeyEvent keyEvent) = 0;
    virtual proto::Command *OnKey(win32::KeyEvent keyEvent) = 0;
    virtual proto::Command *SelectCandidate(int32_t candidate_id) = 0;
    virtual proto::Command *FocusCandidate(int32_t candidate_id) = 0;
    virtual void Reset() = 0;
};

struct EngineControllerFactory {
    static void OnDllProcessAttach(HMODULE module);
    static void OnDllProcessDetach(HMODULE module);
    static void Create(EngineController **ppEngine);
};

} // namespace khiin::win32
