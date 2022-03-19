#include "pch.h"

#include "TextService.h"

#include "proto/proto.h"

#include "CandidateListUI.h"
#include "Compartment.h"
#include "CompositionMgr.h"
#include "CompositionSink.h"
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
        m_threadmgr_sink = make_self<ThreadMgrEventSink>();
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
        m_keyevent_sink->Advise(service);
        m_preservedkeymgr->Initialize(service);

        m_openclose_compartment.Initialize(m_clientid, threadmgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
        m_openclose_compartment.SetValue(true);
        m_openclose_sinkmgr.Advise(m_openclose_compartment.get(), this);
        
        m_config_compartment.Initialize(m_clientid, threadmgr, guids::kConfigChangedCompartment, true);
        m_config_sinkmgr.Advise(m_config_compartment.get(), this);
        m_userdata_compartment.Initialize(m_clientid, threadmgr, guids::kResetUserdataCompartment, true);
        m_userdata_sinkmgr.Advise(m_userdata_compartment.get(), this);

        m_kbd_disabled_compartment.Initialize(m_clientid, threadmgr, GUID_COMPARTMENT_KEYBOARD_DISABLED);
        
        m_engine->Initialize(service);
        InitCategoryMgr();
        InitDisplayAttributes();
        NotifyConfigChangeListeners();
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

    com_ptr<ITfThreadMgr> m_threadmgr = nullptr;
    TfClientId m_clientid = TF_CLIENTID_NULL;
    DWORD m_activate_flags = 0;
    Compartment m_openclose_compartment;
    Compartment m_kbd_disabled_compartment;
    Compartment m_config_compartment;
    Compartment m_userdata_compartment;
    SinkManager<ITfCompartmentEventSink> m_openclose_sinkmgr;
    SinkManager<ITfCompartmentEventSink> m_config_sinkmgr;
    SinkManager<ITfCompartmentEventSink> m_userdata_sinkmgr;
    std::unique_ptr<AppConfig> m_config = nullptr;
    std::vector<ConfigChangeListener *> m_config_listeners;
    std::unique_ptr<PreservedKeyMgr> m_preservedkeymgr = nullptr;

    com_ptr<ITfCategoryMgr> m_categorymgr = nullptr;
    com_ptr<CandidateListUI> m_candidate_list_ui = nullptr;
    com_ptr<CompositionMgr> m_compositionmgr = nullptr;
    com_ptr<DisplayAttributeInfoEnum> m_displayattrs = nullptr;
    com_ptr<ThreadMgrEventSink> m_threadmgr_sink = nullptr;
    com_ptr<KeyEventSink> m_keyevent_sink = nullptr;
    com_ptr<EngineController> m_engine = nullptr;
    com_ptr<LangBarIndicator> m_indicator = nullptr;

    TfGuidAtom m_input_attr = TF_INVALID_GUIDATOM;
    TfGuidAtom m_converted_attr = TF_INVALID_GUIDATOM;
    TfGuidAtom m_focused_attr = TF_INVALID_GUIDATOM;

    //+---------------------------------------------------------------------------
    //
    // TextService
    //
    //----------------------------------------------------------------------------

    virtual HMODULE hmodule() override {
        return g_module;
    }

    virtual TfClientId clientId() override {
        return m_clientid;
    }

    virtual DWORD activateFlags() override {
        return m_activate_flags;
    }

    virtual com_ptr<ITfThreadMgr> thread_mgr() override {
        return m_threadmgr;
    }

    virtual com_ptr<ITfKeystrokeMgr> keystroke_mgr() override {
        return m_threadmgr.as<ITfKeystrokeMgr>();
    }

    virtual com_ptr<CompositionMgr> composition_mgr() override {
        return m_compositionmgr;
    }

    virtual com_ptr<EngineController> engine() override {
        return m_engine;
    }

    virtual com_ptr<CandidateListUI> candidate_ui() override {
        return m_candidate_list_ui;
    }

    virtual AppConfig *config() override {
        return m_config.get();
    }

    virtual winrt::com_ptr<ITfContext> top_context() override {
        KHIIN_TRACE("");
        auto documentMgr = winrt::com_ptr<ITfDocumentMgr>();
        auto context = winrt::com_ptr<ITfContext>();
        check_hresult(m_threadmgr->GetFocus(documentMgr.put()));
        check_hresult(documentMgr->GetTop(context.put()));
        return context;
    }

    virtual winrt::com_ptr<ITfCategoryMgr> category_mgr() override {
        KHIIN_TRACE("");
        return m_categorymgr;
    }

    virtual winrt::com_ptr<ITfCompositionSink> CreateCompositionSink(ITfContext *context) override {
        KHIIN_TRACE("");
        return winrt::make<CompositionSink>(this, context);
    }

    virtual void OnCompositionTerminated(TfEditCookie ecWrite, ITfContext *context,
                                         ITfComposition *pComposition) override {
        KHIIN_TRACE("");
        m_compositionmgr->ClearComposition();
        m_candidate_list_ui->Hide();
    }

    virtual TfGuidAtom input_attribute() {
        return m_input_attr;
    }

    virtual TfGuidAtom converted_attribute() {
        return m_converted_attr;
    }

    virtual TfGuidAtom focused_attribute() {
        return m_focused_attr;
    }

    virtual void OnCandidateSelected(int32_t candidate_id) override {
        auto command = m_engine->SelectCandidate(candidate_id);
        auto context = top_context();
        EditSession::HandleAction(this, context.get(), std::move(command));
    }

    virtual void TipOnOff() {
        TipOnOff(!m_config->ime_enabled().value());
    }

    virtual void TipOnOff(bool on_off) {
        m_config->mutable_ime_enabled()->set_value(on_off);
        NotifyConfigChangeListeners();
    }

    virtual void OnInputModeSelected(proto::InputMode mode) override {
        m_config->set_input_mode(mode);
        NotifyConfigChangeListeners();
    }


    virtual void CycleInputMode() override {
        if (!m_config->ime_enabled().value()) {
            m_config->mutable_ime_enabled()->set_value(true);
        } else {
            switch (m_config->input_mode()) {
            case IM_CONTINUOUS:
                m_config->set_input_mode(IM_BASIC);
                break;
            case IM_BASIC:
                m_config->set_input_mode(IM_PRO);
                break;
            case IM_PRO:
                m_config->set_input_mode(IM_CONTINUOUS);
                break;
            }
        }

        NotifyConfigChangeListeners();
    }

    virtual void OpenSettingsApplication() override {
        SettingsApp::Launch(g_module);
    }

    virtual void RegisterConfigChangeListener(ConfigChangeListener *config_listener) override {
        if (std::find(m_config_listeners.begin(), m_config_listeners.end(), config_listener) ==
            m_config_listeners.end()) {
            m_config_listeners.push_back(config_listener);
        }
    }

    //+---------------------------------------------------------------------------
    //
    // ITfTextInputProcessorEx
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid) override {
        KHIIN_TRACE("");
        return ActivateEx(ptim, tid, 0);
    }

    virtual STDMETHODIMP Deactivate(void) override {
        KHIIN_TRACE("");
        TRY_FOR_HRESULT;

        OnDeactivate();

        m_threadmgr = nullptr;
        m_clientid = TF_CLIENTID_NULL;
        m_activate_flags = 0;

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tid, DWORD dwFlags) override {
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

    virtual STDMETHODIMP OnSetThreadFocus(void) override {
        KHIIN_TRACE("");
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP OnKillThreadFocus(void) override {
        KHIIN_TRACE("");
        return E_NOTIMPL;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfTextLayoutSink
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pView) {
        KHIIN_TRACE("");
        return E_NOTIMPL;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfDisplayAttributeProvider
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum) override {
        TRY_FOR_HRESULT;
        m_displayattrs.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) override {
        TRY_FOR_HRESULT;
        m_displayattrs->findByGuid(guid, ppInfo);
        CATCH_FOR_HRESULT;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCompartmentEventSink
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP OnChange(REFGUID rguid) override {
        KHIIN_TRACE("");
        TRY_FOR_HRESULT;
        auto hr = E_FAIL;

        if (rguid == GUID_COMPARTMENT_KEYBOARD_OPENCLOSE) {
            DWORD val;
            m_openclose_compartment.GetValue(&val);

            if (val == false) {
                m_engine->Reset();
                m_candidate_list_ui->DestroyCandidateWindow();
            }
        }

        if (rguid == guids::kConfigChangedCompartment) {
            InitConfig();
            NotifyConfigChangeListeners();
        }

        if (rguid == guids::kResetUserdataCompartment) {
            auto cmd = Command();
            cmd.mutable_request()->set_type(CMD_RESET_USER_DATA);
            m_engine->SendCommand(std::move(cmd));
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
