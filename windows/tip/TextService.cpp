#include "pch.h"

#include "TextService.h"

#include "proto/proto.h"

#include "CandidateListUI.h"
#include "Compartment.h"
#include "CompositionMgr.h"
#include "CompositionSink.h"
#include "CompositionUtil.h"
#include "Config.h"
#include "DisplayAttributeInfoEnum.h"
#include "DllModule.h"
#include "EditSession.h"
#include "EngineController.h"
#include "Guids.h"
#include "KeyEventSink.h"
#include "LangBarIndicator.h"
#include "PreservedKeyMgr.h"
#include "SettingsApp.h"
#include "TextEditSink.h"
#include "ThreadMgrEventSink.h"
#include "Utils.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;
using namespace proto;

volatile HMODULE g_module = nullptr;

struct TextServiceImpl :
    winrt::implements<TextServiceImpl, // clang-format off
                      ITfTextInputProcessorEx,
                      ITfDisplayAttributeProvider,
                      ITfThreadFocusSink,
                      ITfTextLayoutSink,
                      ITfCompartmentEventSink,
                      TextService> { // clang-format on
    TextServiceImpl() {
        m_engine = EngineController::Create();
        m_compositionmgr = CompositionMgr::Create();
        m_threadmgr_sink = ThreadMgrEventSink::Create();
        m_candidate_list_ui = CandidateListUI::Create();
        m_keyevent_sink = KeyEventSink::Create();
        m_indicator = LangBarIndicator::Create();
        m_preservedkeymgr = PreservedKeyMgr::Create();
    }

  private:
    HRESULT OnActivate() {
        auto hr = E_FAIL;
        auto service = cast_as<TextService>(this);
        auto threadmgr = m_threadmgr.get();

        InitConfig();
        DisplayAttributeInfoEnum::load(m_displayattrs.put());
        m_indicator->Initialize(service);
        m_compositionmgr->Initialize(service);
        m_threadmgr_sink->Initialize(service);
        m_candidate_list_ui->Initialize(service);
        m_openclose_compartment.Initialize(m_clientid, threadmgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
        m_openclose_compartment.SetValue(true);
        m_openclose_sinkmgr.Advise(m_openclose_compartment.get(), this);
        m_config_compartment.Initialize(m_clientid, threadmgr, guids::kConfigChangedCompartment, true);
        m_config_sinkmgr.Advise(m_config_compartment.get(), this);
        m_userdata_compartment.Initialize(m_clientid, threadmgr, guids::kResetUserdataCompartment, true);
        m_userdata_sinkmgr.Advise(m_userdata_compartment.get(), this);
        m_kbd_disabled_compartment.Initialize(m_clientid, threadmgr, GUID_COMPARTMENT_KEYBOARD_DISABLED);
        m_engine->Initialize(service);
        m_keyevent_sink->Advise(service);
        m_preservedkeymgr->Initialize(service);
        InitCategoryMgr();
        InitDisplayAttributes();
        NotifyConfigChangeListeners();
        SetEnabled(true);
        return S_OK;
    }

    HRESULT OnDeactivate() {
        auto hr = E_FAIL;
        m_engine->Uninitialize();
        m_keyevent_sink->Unadvise();
        m_openclose_sinkmgr.Unadvise();
        m_openclose_compartment.SetValue(false);
        m_openclose_compartment.Uninitialize();
        m_config_sinkmgr.Unadvise();
        m_config_compartment.Uninitialize();
        m_userdata_sinkmgr.Unadvise();
        m_userdata_compartment.Uninitialize();
        m_kbd_disabled_compartment.Uninitialize();
        m_candidate_list_ui->Uninitialize();
        m_preservedkeymgr->Shutdown();
        m_threadmgr_sink->Uninitialize();
        m_compositionmgr->Uninitialize();
        m_indicator->Shutdown();
        m_displayattrs = nullptr;
        m_categorymgr = nullptr;
        m_config = nullptr;
        m_config_listeners.clear();
        return S_OK;
    }

    void InitCategoryMgr() {
        check_hresult(::CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,
                                         m_categorymgr.put_void()));
    }

    void InitDisplayAttributes() {
        check_hresult(m_categorymgr->RegisterGUID(DisplayAttribute_Input.guid, &m_input_attr));
        check_hresult(m_categorymgr->RegisterGUID(DisplayAttribute_Converted.guid, &m_converted_attr));
        check_hresult(m_categorymgr->RegisterGUID(DisplayAttribute_Focused.guid, &m_focused_attr));
    }

    void InitConfig() {
        if (!m_config) {
            m_config = std::make_unique<AppConfig>();
        }
        Config::LoadFromFile(g_module, m_config.get());
    }

    void NotifyConfigChangeListeners() {
        for (auto &listener : m_config_listeners) {
            if (listener) {
                listener->OnConfigChanged(m_config.get());
            }
        }
    }

    //+---------------------------------------------------------------------------
    //
    // TextService
    //
    //----------------------------------------------------------------------------

    HMODULE hmodule() override {
        return g_module;
    }

    TfClientId client_id() override {
        return m_clientid;
    }

    DWORD activate_flags() override {
        return m_activate_flags;
    }

    AppConfig *config() override {
        return m_config.get();
    }

    com_ptr<ITfThreadMgr> thread_mgr() override {
        return m_threadmgr;
    }

    com_ptr<ITfKeystrokeMgr> keystroke_mgr() override {
        return m_threadmgr.as<ITfKeystrokeMgr>();
    }

    com_ptr<CompositionMgr> composition_mgr() override {
        return m_compositionmgr;
    }

    com_ptr<EngineController> engine() override {
        return m_engine;
    }

    com_ptr<CandidateListUI> candidate_ui() override {
        return m_candidate_list_ui;
    }

    com_ptr<ITfContext> context() override {
        KHIIN_TRACE("");
        if (m_context) {
            return m_context;
        }

        auto documentMgr = winrt::com_ptr<ITfDocumentMgr>();
        auto base_context = winrt::com_ptr<ITfContext>();
        check_hresult(m_threadmgr->GetFocus(documentMgr.put()));
        check_hresult(documentMgr->GetBase(base_context.put()));
        return base_context;
    }

    winrt::com_ptr<ITfCategoryMgr> category_mgr() override {
        KHIIN_TRACE("");
        return m_categorymgr;
    }

    TfGuidAtom DisplayAttributeAtom(SegmentStatus status) override {
        switch (status) {
        case SegmentStatus::Composing:
            return m_input_attr;
        case SegmentStatus::Focused:
            return m_focused_attr;
        case SegmentStatus::Converted:
            return m_converted_attr;
        default:
            return TF_INVALID_GUIDATOM;
        }
    }

    winrt::com_ptr<ITfCompositionSink> CreateCompositionSink(ITfContext *context) override {
        KHIIN_TRACE("");
        return winrt::make<CompositionSink>(this, context);
    }

    void OnCompositionTerminated(TfEditCookie ecWrite, ITfContext *context, ITfComposition *pComposition) override {
        KHIIN_TRACE("");
        KHIIN_DEBUG("OnCompositionTerminated");
        m_engine->Reset();
        pComposition->EndComposition(ecWrite);
        m_compositionmgr->ClearComposition(ecWrite);
        m_candidate_list_ui->Hide();
    }

    /**
     * Khiin IME can only handle one context at a time.
     * A new context is first received in |ThreadMgrEventSink::OnInitDocumentMgr|
     * and stored in the TextService while it is active.
     * We also test if the context has changed in |KeyEventSink|, in case
     * |OnInitDocumentMgr| was not called.
     */
    bool UpdateContext(ITfContext *context) override {
        if (!m_context && context == nullptr) {
            return false;
        }

        if (m_context && context == nullptr) {
            m_context = nullptr;
            return true;
        }

        if (m_context && context == m_context.get()) {
            return false;
        }

        WINRT_ASSERT(context != nullptr);
        m_context.copy_from(context);
        EditSession::HandleFocusChange(this, context);
        return true;
    }

    void OnCandidateSelected(int32_t candidate_id) override {
        auto command = m_engine->SelectCandidate(candidate_id);
        EditSession::HandleAction(this, context().get(), command);
    }

    void UpdateCandidateWindow(TfEditCookie cookie) override {
        m_candidate_list_ui->Move(cookie);
    }

    void CommitComposition() override {
        auto command = m_engine->Commit();
        EditSession::HandleAction(this, context().get(), command);
    }

    void Reset() override {
        m_engine->Reset();
    }

    bool Enabled() override {
        return m_config->ime_enabled().value();
    }

    void TipOnOff() override {
        SetEnabled(!m_config->ime_enabled().value());
    }

    void SetEnabled(bool on_off) override {
        KHIIN_TRACE("");
        if (m_on_off_lock) {
            return;
        }

        if (m_config->ime_enabled().value() != on_off) {
            KHIIN_TRACE("Setting enabled: {}", on_off);
            m_config->mutable_ime_enabled()->set_value(on_off);
            NotifyConfigChangeListeners();
        }

        if (!on_off) {
            auto command = m_engine->Commit();
            EditSession::HandleAction(this, context().get(), command);
        }
    }

    void SetLocked(bool locked) override {
        KHIIN_TRACE("");
        if (locked) {
            SetEnabled(false);
            m_on_off_lock = true;
        } else {
            m_on_off_lock = false;
            SetEnabled(true);
        }
    }

    void OnInputModeSelected(proto::InputMode mode) override {
        m_config->set_input_mode(mode);
        NotifyConfigChangeListeners();
    }

    void CycleInputMode() override {
        Config::CycleInputMode(m_config.get());
        NotifyConfigChangeListeners();
    }

    void OpenSettingsApplication() override {
        SettingsApp::Launch(g_module);
    }

    void RegisterConfigChangeListener(ConfigChangeListener *config_listener) override {
        if (std::find(m_config_listeners.begin(), m_config_listeners.end(), config_listener) ==
            m_config_listeners.end()) {
            m_config_listeners.push_back(config_listener);
        }
    }

    // Provided by TSF in |ITfTextInputProcessorEx::ActivateEx|
    com_ptr<ITfThreadMgr> m_threadmgr = nullptr;
    TfClientId m_clientid = TF_CLIENTID_NULL;
    DWORD m_activate_flags = 0;

    // Compartments and comparment sinks (event listeners)
    Compartment m_openclose_compartment;
    Compartment m_kbd_disabled_compartment;
    Compartment m_config_compartment;
    Compartment m_userdata_compartment;
    SinkManager<ITfCompartmentEventSink> m_openclose_sinkmgr;
    SinkManager<ITfCompartmentEventSink> m_config_sinkmgr;
    SinkManager<ITfCompartmentEventSink> m_userdata_sinkmgr;

    std::unique_ptr<AppConfig> m_config = nullptr;
    std::vector<ConfigChangeListener *> m_config_listeners = {};
    std::unique_ptr<PreservedKeyMgr> m_preservedkeymgr = nullptr;

    com_ptr<ITfCategoryMgr> m_categorymgr = nullptr;
    com_ptr<CandidateListUI> m_candidate_list_ui = nullptr;
    com_ptr<CompositionMgr> m_compositionmgr = nullptr;
    com_ptr<DisplayAttributeInfoEnum> m_displayattrs = nullptr;
    com_ptr<ThreadMgrEventSink> m_threadmgr_sink = nullptr;
    com_ptr<KeyEventSink> m_keyevent_sink = nullptr;
    com_ptr<EngineController> m_engine = nullptr;
    com_ptr<LangBarIndicator> m_indicator = nullptr;
    com_ptr<ITfContext> m_context = nullptr;

    TfGuidAtom m_input_attr = TF_INVALID_GUIDATOM;
    TfGuidAtom m_converted_attr = TF_INVALID_GUIDATOM;
    TfGuidAtom m_focused_attr = TF_INVALID_GUIDATOM;
    bool m_on_off_lock = false;

    //+---------------------------------------------------------------------------
    //
    // ITfTextInputProcessorEx
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid) override {
        KHIIN_TRACE("");
        return ActivateEx(ptim, tid, 0);
    }

    STDMETHODIMP Deactivate(void) override {
        KHIIN_TRACE("");
        TRY_FOR_HRESULT;

        OnDeactivate();

        m_threadmgr = nullptr;
        m_clientid = TF_CLIENTID_NULL;
        m_activate_flags = 0;

        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tid, DWORD dwFlags) override {
        KHIIN_INFO("Activating Text Service, client_id {}, flags {:x}", tid, dwFlags);
        TRY_FOR_HRESULT;

        m_threadmgr.copy_from(pThreadMgr);
        m_clientid = tid;
        m_activate_flags = dwFlags;
        return OnActivate();

        CATCH_FOR_HRESULT;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfThreadFocusSink
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP OnSetThreadFocus(void) override {
        KHIIN_TRACE("");
        return E_NOTIMPL;
    }

    STDMETHODIMP OnKillThreadFocus(void) override {
        KHIIN_TRACE("");
        return E_NOTIMPL;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfTextLayoutSink
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP OnLayoutChange(ITfContext *context, TfLayoutCode lcode, ITfContextView *pView) {
        KHIIN_TRACE("");
        return E_NOTIMPL;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfDisplayAttributeProvider
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum) override {
        TRY_FOR_HRESULT;
        m_displayattrs.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) override {
        TRY_FOR_HRESULT;
        m_displayattrs->findByGuid(guid, ppInfo);
        CATCH_FOR_HRESULT;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCompartmentEventSink
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP OnChange(REFGUID rguid) override {
        KHIIN_TRACE("");
        TRY_FOR_HRESULT;
        auto hr = E_FAIL;

        if (rguid == GUID_COMPARTMENT_KEYBOARD_OPENCLOSE) {
            DWORD val{};
            m_openclose_compartment.GetValue(&val);

            if (val == 0) {
                m_engine->Reset();
                m_candidate_list_ui->DestroyCandidateWindow();
            }

            SetEnabled(true);
        }

        if (rguid == guids::kConfigChangedCompartment) {
            InitConfig();
            NotifyConfigChangeListeners();
        }

        if (rguid == guids::kResetUserdataCompartment) {
            m_engine->ResetUserData();
        }

        CATCH_FOR_HRESULT;
    }
};

} // namespace

void TextService::OnDllProcessAttach(HMODULE module) {
    DllModule::AddRef();
    g_module = module;
}

void TextService::OnDllProcessDetach(HMODULE module) {
    g_module = nullptr;
    DllModule::Release();
}

com_ptr<TextService> TextService::Create() {
    return as_self<TextService>(winrt::make_self<TextServiceImpl>());
}

} // namespace khiin::win32::tip
