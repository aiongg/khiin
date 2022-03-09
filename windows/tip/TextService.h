#pragma once

#include "pch.h"

#include "common.h"

#include "KeyEvent.h"

namespace khiin::proto {
enum InputMode : int;
class AppConfig;
}

namespace khiin::win32 {

struct EngineController;
struct CandidateListUI;
struct ConfigChangeListener;

struct TextService : winrt::implements<TextService, IUnknown> {
    TextService() = default;
    TextService(const TextService &) = delete;
    TextService &operator=(const TextService &) = delete;
    ~TextService() = default;

    virtual HMODULE hmodule() = 0;
    virtual TfClientId clientId() = 0;
    virtual DWORD activateFlags() = 0;

    virtual ITfThreadMgr *thread_mgr() = 0;
    virtual IUnknown *composition_mgr() = 0;
    virtual IEnumTfDisplayAttributeInfo *displayAttrInfoEnum() = 0;
    virtual EngineController *engine() = 0;
    virtual CandidateListUI *candidate_ui() = 0;
    virtual proto::AppConfig *config() = 0;

    virtual winrt::com_ptr<ITfCategoryMgr> categoryMgr() = 0;
    virtual winrt::com_ptr<ITfContext> GetTopContext() = 0;
    virtual winrt::com_ptr<ITfCompositionSink> CreateCompositionSink(ITfContext *context) = 0;
    virtual void OnCompositionTerminated(TfEditCookie ecWrite, ITfContext *context, ITfComposition *pComposition) = 0;
    virtual void OnCandidateSelected(int32_t candidate_id) = 0;

    virtual void OnInputModeSelected(proto::InputMode mode) = 0;
    virtual void OpenSettingsApplication() = 0;

    virtual TfGuidAtom input_attribute() = 0;
    virtual TfGuidAtom converted_attribute() = 0;
    virtual TfGuidAtom focused_attribute() = 0;

    virtual void RegisterConfigChangeListener(ConfigChangeListener *config_listener) = 0;
};

struct TextServiceFactory {
    static void OnDllProcessAttach(HMODULE module);
    static void OnDllProcessDetach(HMODULE module);
    static void Create(TextService **ppService);
};

} // namespace khiin::win32
