#pragma once

#include "KeyEvent.h"

namespace khiin::proto {
class Command;
class CandidateList;
}

namespace khiin::win32::tip {

struct TextService;

struct EngineController : winrt::implements<EngineController, IUnknown> {
    EngineController() = default;
    EngineController(const EngineController &) = delete;
    EngineController &operator=(const EngineController &) = delete;
    ~EngineController() = default;

    static void OnDllProcessAttach(HMODULE module);
    static void OnDllProcessDetach(HMODULE module);
    static winrt::com_ptr<EngineController> Create();

    virtual void Initialize(TextService *service) = 0;
    virtual void Uninitialize() = 0;

    virtual proto::Command *TestKey(win32::KeyEvent keyEvent) = 0;
    virtual proto::Command *OnKey(win32::KeyEvent keyEvent) = 0;
    virtual proto::Command *SelectCandidate(int32_t candidate_id) = 0;
    virtual proto::Command *FocusCandidate(int32_t candidate_id) = 0;
    virtual proto::Command *Commit() = 0;
    virtual void ResetUserData() = 0;
    virtual void Reset() = 0;
    virtual proto::CandidateList *LoadEmojis() = 0;
};

} // namespace khiin::win32
