#include "pch.h"

#include "TextService.h"

#include "CandidateListUI.h"
#include "Compartment.h"
#include "CompositionMgr.h"
#include "CompositionSink.h"
#include "Config.h"
#include "DisplayAttributeInfoEnum.h"
#include "DllModule.h"
#include "EditSession.h"
#include "EngineController.h"
#include "KeyEventSink.h"
#include "TextEditSink.h"
#include "ThreadMgrEventSink.h"
#include "common.h"

#include "Utils.h"

namespace khiin::win32 {
namespace {
using namespace winrt;
using namespace messages;

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
        EngineControllerFactory::Create(m_engine.put());
        m_compositionmgr = make_self<CompositionMgr>();
        m_threadmgr_sink = make_self<ThreadMgrEventSink>();
        CandidateListUIFactory::Create(m_candidate_list_ui.put());
        m_keyevent_sink = make_self<KeyEventSink>();
    }

  private:
    HRESULT OnActivate() {
        auto hr = E_FAIL;
        auto pTextService = cast_as<TextService>(this);
        InitConfig();
        DisplayAttributeInfoEnum::load(m_displayattrs.put());
        m_compositionmgr->Initialize(pTextService);
        m_threadmgr_sink->Initialize(pTextService);
        m_candidate_list_ui->Initialize(pTextService);
        m_keyevent_sink->Activate(pTextService);
        m_openclose_compartment.Initialize(m_clientid, m_threadmgr.get(), GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
        m_kbd_disabled_compartment.Initialize(m_clientid, m_threadmgr.get(), GUID_COMPARTMENT_KEYBOARD_DISABLED);
        m_config_compartment.Initialize(m_clientid, m_threadmgr.get(), kConfigChangedCompartmentGuid, true);
        m_openclose_compartment.SetValue(true);
        m_openclose_sinkmgr.Advise(m_openclose_compartment.getCompartment(), this);
        m_config_sinkmgr.Advise(m_config_compartment.getCompartment(), this);
        m_engine->Initialize();
        InitCategoryMgr();
        InitDisplayAttributes();
        return S_OK;
    }

    HRESULT OnDeactivate() {
        auto hr = E_FAIL;
        m_engine->Uninitialize();
        m_openclose_sinkmgr.Unadvise();
        m_openclose_compartment.SetValue(false);
        m_kbd_disabled_compartment.Uninitialize();
        m_openclose_compartment.Uninitialize();
        m_candidate_list_ui->Uninitialize();
        m_keyevent_sink->Deactivate();
        m_threadmgr_sink->Uninitialize();
        m_compositionmgr->Uninitialize();
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
            m_config = AppConfig::default_instance().New();
        }

        Config::LoadFromFile(g_module, m_config);
    }

    void OnConfigChanged() {
        InitConfig();
        for (auto &listener : m_config_listeners) {
            if (listener) {
                listener->OnConfigChanged(m_config);
            }
        }
    }

    com_ptr<ITfThreadMgr> m_threadmgr = nullptr;
    TfClientId m_clientid = TF_CLIENTID_NULL;
    DWORD m_activate_flags = 0;
    Compartment m_openclose_compartment;
    Compartment m_kbd_disabled_compartment;
    Compartment m_config_compartment;
    SinkManager<ITfCompartmentEventSink> m_openclose_sinkmgr;
    SinkManager<ITfCompartmentEventSink> m_config_sinkmgr;
    AppConfig *m_config = nullptr;
    std::vector<ConfigChangeListener *> m_config_listeners;

    com_ptr<ITfCategoryMgr> m_categorymgr = nullptr;
    com_ptr<CandidateListUI> m_candidate_list_ui = nullptr;
    com_ptr<CompositionMgr> m_compositionmgr = nullptr;
    com_ptr<DisplayAttributeInfoEnum> m_displayattrs = nullptr;
    com_ptr<ThreadMgrEventSink> m_threadmgr_sink = nullptr;
    com_ptr<KeyEventSink> m_keyevent_sink = nullptr;
    com_ptr<EngineController> m_engine = nullptr;

    TfGuidAtom m_input_attr = TF_INVALID_GUIDATOM;
    TfGuidAtom m_converted_attr = TF_INVALID_GUIDATOM;
    TfGuidAtom m_focused_attr = TF_INVALID_GUIDATOM;


  public:
    //+---------------------------------------------------------------------------
    //
    // TextService
    //
    //----------------------------------------------------------------------------

    virtual TfClientId clientId() override {
        D(__FUNCTIONW__);
        return m_clientid;
    }

    virtual DWORD activateFlags() override {
        D(__FUNCTIONW__);
        return m_activate_flags;
    }

    virtual ITfThreadMgr *thread_mgr() override {
        D(__FUNCTIONW__);
        return m_threadmgr.get();
    }

    virtual IUnknown *composition_mgr() override {
        D(__FUNCTIONW__);
        return m_compositionmgr.as<IUnknown>().get();
    }

    virtual IEnumTfDisplayAttributeInfo *displayAttrInfoEnum() override {
        D(__FUNCTIONW__);
        IEnumTfDisplayAttributeInfo *tmp = nullptr;
        EnumDisplayAttributeInfo(&tmp);
        return tmp;
    }

    virtual EngineController *engine() override {
        D(__FUNCTIONW__);
        return m_engine.get();
    }

    virtual CandidateListUI *candidate_ui() override {
        D(__FUNCTIONW__);
        return m_candidate_list_ui.get();
    }

    virtual AppConfig *config() override {
        return m_config;
    }

    virtual winrt::com_ptr<ITfContext> GetTopContext() override {
        D(__FUNCTIONW__);
        auto documentMgr = winrt::com_ptr<ITfDocumentMgr>();
        auto context = winrt::com_ptr<ITfContext>();
        check_hresult(m_threadmgr->GetFocus(documentMgr.put()));
        check_hresult(documentMgr->GetTop(context.put()));
        return context;
    }

    virtual winrt::com_ptr<ITfCategoryMgr> categoryMgr() override {
        D(__FUNCTIONW__);
        return m_categorymgr;
    }

    virtual winrt::com_ptr<ITfCompositionSink> CreateCompositionSink(ITfContext *context) override {
        D(__FUNCTIONW__);
        return winrt::make<CompositionSink>(this, context);
    }

    virtual void OnCompositionTerminated(TfEditCookie ecWrite, ITfContext *context,
                                         ITfComposition *pComposition) override {
        D(__FUNCTIONW__);
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
        auto context = GetTopContext();
        EditSession::HandleAction(this, context.get(), std::move(command));
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
        D(__FUNCTIONW__);
        return ActivateEx(ptim, tid, 0);
    }

    virtual STDMETHODIMP Deactivate(void) override {
        D(__FUNCTIONW__);
        TRY_FOR_HRESULT;

        OnDeactivate();

        m_threadmgr = nullptr;
        m_clientid = TF_CLIENTID_NULL;
        m_activate_flags = 0;

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tid, DWORD dwFlags) override {
        D(__FUNCTIONW__, L" clientId: ", tid);
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
        D(__FUNCTIONW__);
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP OnKillThreadFocus(void) override {
        D(__FUNCTIONW__);
        return E_NOTIMPL;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfTextLayoutSink
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pView) {
        D(__FUNCTIONW__);
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
        D(__FUNCTIONW__);
        TRY_FOR_HRESULT;
        auto hr = E_FAIL;

        if (rguid == GUID_COMPARTMENT_KEYBOARD_OPENCLOSE) {
            DWORD val;
            m_openclose_compartment.GetValue(&val);

            if (val == false) {
                m_engine->Reset();
                m_candidate_list_ui->DestroyCandidateWindow();
            }
        } else if (rguid == kConfigChangedCompartmentGuid) {
            OnConfigChanged();
        }

        CATCH_FOR_HRESULT;
    }
};

} // namespace

//+---------------------------------------------------------------------------
//
// TextServiceFactory
//
//----------------------------------------------------------------------------

void TextServiceFactory::OnDllProcessAttach(HMODULE module) {
    DllModule::AddRef();
    g_module = module;
}

void TextServiceFactory::OnDllProcessDetach(HMODULE module) {
    g_module = nullptr;
    DllModule::Release();
}

void TextServiceFactory::Create(TextService **ppService) {
    D(__FUNCTIONW__);
    as_self<TextService>(winrt::make_self<TextServiceImpl>()).copy_to(ppService);
}

} // namespace khiin::win32
