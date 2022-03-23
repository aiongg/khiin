#pragma once

#include "pch.h"

namespace khiin::proto {
enum InputMode : int;
class AppConfig;
} // namespace khiin::proto

namespace khiin::win32::tip {

struct CandidateListUI;
struct CompositionMgr;
struct ConfigChangeListener;
struct EngineController;
enum class SegmentStatus;

struct TextService : winrt::implements<TextService, IUnknown> {
    TextService() = default;
    TextService(const TextService &) = delete;
    TextService &operator=(const TextService &) = delete;
    ~TextService() = default;

    static void OnDllProcessAttach(HMODULE module);
    static void OnDllProcessDetach(HMODULE module);
    static winrt::com_ptr<TextService> Create();

    virtual HMODULE hmodule() = 0;
    virtual TfClientId client_id() = 0;
    virtual DWORD activate_flags() = 0;

    virtual winrt::com_ptr<ITfThreadMgr> thread_mgr() = 0;
    virtual winrt::com_ptr<ITfKeystrokeMgr> keystroke_mgr() = 0;
    virtual winrt::com_ptr<ITfCategoryMgr> category_mgr() = 0;
    virtual winrt::com_ptr<ITfContext> context() = 0;
    virtual winrt::com_ptr<EngineController> engine() = 0;
    virtual winrt::com_ptr<CompositionMgr> composition_mgr() = 0;
    virtual winrt::com_ptr<CandidateListUI> candidate_ui() = 0;
    virtual winrt::com_ptr<ITfCompositionSink> CreateCompositionSink(ITfContext *context) = 0;

    virtual proto::AppConfig *config() = 0;

    virtual void Reset() = 0;
    virtual void CommitComposition() = 0;
    virtual void UpdateCandidateWindow(TfEditCookie cookie) = 0;
    virtual bool UpdateContext(ITfContext *context) = 0;
    virtual bool Enabled() = 0;
    virtual void SetEnabled(bool enable) = 0;
    virtual void TipOnOff() = 0;
    virtual void SetLocked(bool lock) = 0;
    virtual void OnCompositionTerminated(TfEditCookie ecWrite, ITfContext *context, ITfComposition *pComposition) = 0;
    virtual void OnCandidateSelected(int32_t candidate_id) = 0;
    virtual void OnInputModeSelected(proto::InputMode mode) = 0;
    virtual void CycleInputMode() = 0;
    virtual void OpenSettingsApplication() = 0;
    virtual void RegisterConfigChangeListener(ConfigChangeListener *config_listener) = 0;

    virtual TfGuidAtom DisplayAttributeAtom(SegmentStatus status) = 0;
};

} // namespace khiin::win32::tip
