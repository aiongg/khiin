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

struct TextService : winrt::implements<TextService, IUnknown> {
    TextService() = default;
    TextService(const TextService &) = delete;
    TextService &operator=(const TextService &) = delete;
    ~TextService() = default;

    static void OnDllProcessAttach(HMODULE module);
    static void OnDllProcessDetach(HMODULE module);
    static winrt::com_ptr<TextService> Create();

    virtual HMODULE hmodule() = 0;
    virtual TfClientId clientId() = 0;
    virtual DWORD activateFlags() = 0;

    virtual winrt::com_ptr<ITfThreadMgr> thread_mgr() = 0;
    virtual winrt::com_ptr<ITfKeystrokeMgr> keystroke_mgr() = 0;
    virtual winrt::com_ptr<ITfCategoryMgr> category_mgr() = 0;
    virtual winrt::com_ptr<ITfContext> top_context() = 0;
    virtual winrt::com_ptr<EngineController> engine() = 0;
    virtual winrt::com_ptr<CompositionMgr> composition_mgr() = 0;
    virtual winrt::com_ptr<CandidateListUI> candidate_ui() = 0;
    virtual winrt::com_ptr<ITfCompositionSink> CreateCompositionSink(ITfContext *context) = 0;

    virtual proto::AppConfig *config() = 0;

    virtual void OnCompositionTerminated(TfEditCookie ecWrite, ITfContext *context, ITfComposition *pComposition) = 0;
    virtual void OnCandidateSelected(int32_t candidate_id) = 0;
    virtual void OnInputModeSelected(proto::InputMode mode) = 0;
    virtual void OpenSettingsApplication() = 0;
    virtual void RegisterConfigChangeListener(ConfigChangeListener *config_listener) = 0;
    virtual void SwapOnOff() = 0;

    virtual TfGuidAtom input_attribute() = 0;
    virtual TfGuidAtom converted_attribute() = 0;
    virtual TfGuidAtom focused_attribute() = 0;
};

} // namespace khiin::win32::tip
