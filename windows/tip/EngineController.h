#pragma once

#include "KeyEvent.h"
#include "common.h"

namespace khiin::win32 {

struct EngineController : winrt::implements<EngineController, IUnknown> {
    EngineController() = default;
    EngineController(const EngineController &) = delete;
    EngineController &operator=(const EngineController &) = delete;
    ~EngineController() = default;

    virtual void Initialize() = 0;
    virtual void Uninitialize() = 0;

    virtual proto::Command TestKey(KeyEvent keyEvent) = 0;
    virtual proto::Command OnKey(KeyEvent keyEvent) = 0;
    virtual proto::Command SelectCandidate(int32_t candidate_id) = 0;
    virtual proto::Command FocusCandidate(int32_t candidate_id) = 0;
    virtual void Reset() = 0;
};

struct EngineControllerFactory {
    static void OnDllProcessAttach(HMODULE module);
    static void OnDllProcessDetach(HMODULE module);
    static void Create(EngineController **ppEngine);
};

} // namespace khiin::win32
