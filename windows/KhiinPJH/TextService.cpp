#include "pch.h"

#include "TextService.h"

#include "CandidateListUI.h"
#include "Compartment.h"
#include "CompositionMgr.h"
#include "CompositionSink.h"
#include "DisplayAttributeInfoEnum.h"
#include "EditSession.h"
#include "KeyEventSink.h"
#include "TextEditSink.h"
#include "ThreadMgrEventSink.h"
#include "common.h"

namespace khiin::win32 {

struct TextServiceImpl :
    winrt::implements<TextServiceImpl, // clang-format off
                      ITfTextInputProcessorEx,
                      ITfDisplayAttributeProvider,
                      ITfThreadFocusSink,
                      ITfTextLayoutSink,
                      ITfCompartmentEventSink,
                      TextService> { // clang-format on
    TextServiceImpl() {
        TextEngineFactory::Create(engine_.put());
        compositionMgr_ = winrt::make_self<CompositionMgr>();
        threadMgrEventSink_ = winrt::make_self<ThreadMgrEventSink>();
        candidateListUI_ = winrt::make_self<CandidateListUI>();
        keyEventSink_ = winrt::make_self<KeyEventSink>();
    }

  private:
    HRESULT OnActivate() {
        auto hr = E_FAIL;
        auto pTextService = cast_as<TextService>(this);
        DisplayAttributeInfoEnum::load(displayAttributes_.put());
        compositionMgr_->Initialize(pTextService);
        threadMgrEventSink_->Initialize(pTextService);
        candidateListUI_->Initialize(pTextService);
        keyEventSink_->Activate(pTextService);
        openCloseCompartment_.Initialize(clientId_, threadMgr_.get(), GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
        keyboardDisabledCompartment_.Initialize(clientId_, threadMgr_.get(), GUID_COMPARTMENT_KEYBOARD_DISABLED);
        openCloseCompartment_.SetValue(true);
        openCloseSinkMgr_.Advise(openCloseCompartment_.getCompartment(), this);
        engine_->Initialize();
        return S_OK;
    }

    HRESULT OnDeactivate() {
        auto hr = E_FAIL;
        engine_->Uninitialize();
        openCloseSinkMgr_.Unadvise();
        openCloseCompartment_.SetValue(false);
        keyboardDisabledCompartment_.Uninitialize();
        openCloseCompartment_.Uninitialize();
        candidateListUI_->Uninitialize();
        keyEventSink_->Deactivate();
        threadMgrEventSink_->Uninitialize();
        compositionMgr_->Uninitialize();
        displayAttributes_ = nullptr;
        return S_OK;
    }

    winrt::com_ptr<ITfThreadMgr> threadMgr_ = nullptr;
    TfClientId clientId_ = TF_CLIENTID_NULL;
    DWORD activateFlags_ = 0;
    Compartment openCloseCompartment_;
    Compartment keyboardDisabledCompartment_;
    SinkManager<ITfCompartmentEventSink> openCloseSinkMgr_;

    winrt::com_ptr<CandidateListUI> candidateListUI_ = nullptr;
    winrt::com_ptr<CompositionMgr> compositionMgr_ = nullptr;
    winrt::com_ptr<DisplayAttributeInfoEnum> displayAttributes_ = nullptr;
    winrt::com_ptr<ThreadMgrEventSink> threadMgrEventSink_ = nullptr;
    winrt::com_ptr<KeyEventSink> keyEventSink_ = nullptr;
    winrt::com_ptr<TextEngine> engine_ = nullptr;

  public:
    //+---------------------------------------------------------------------------
    //
    // TextService
    //
    //----------------------------------------------------------------------------

    virtual TfClientId clientId() override {
        D(__FUNCTIONW__);
        return clientId_;
    }

    virtual DWORD activateFlags() override {
        D(__FUNCTIONW__);
        return activateFlags_;
    }

    virtual ITfThreadMgr *threadMgr() override {
        D(__FUNCTIONW__);
        return threadMgr_.get();
    }

    virtual IUnknown *compositionMgr() override {
        D(__FUNCTIONW__);
        return compositionMgr_.as<IUnknown>().get();
    }

    virtual IEnumTfDisplayAttributeInfo *displayAttrInfoEnum() override {
        D(__FUNCTIONW__);
        IEnumTfDisplayAttributeInfo *tmp = nullptr;
        EnumDisplayAttributeInfo(&tmp);
        return tmp;
    }

    virtual TextEngine *engine() override {
        D(__FUNCTIONW__);
        return engine_.get();
    }

    virtual ITfUIElement *candidateUI() override {
        D(__FUNCTIONW__);
        return candidateListUI_.as<ITfUIElement>().get();
    }

    virtual winrt::com_ptr<ITfContext> GetTopContext() override {
        D(__FUNCTIONW__);
        auto documentMgr = winrt::com_ptr<ITfDocumentMgr>();
        auto context = winrt::com_ptr<ITfContext>();
        winrt::check_hresult(threadMgr_->GetFocus(documentMgr.put()));
        winrt::check_hresult(documentMgr->GetTop(context.put()));
        return context;
    }

    virtual winrt::com_ptr<ITfCategoryMgr> categoryMgr() override {
        D(__FUNCTIONW__);
        auto tmp = winrt::com_ptr<ITfCategoryMgr>();
        winrt::check_hresult(
            ::CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, tmp.put_void()));
        return tmp;
    }

    virtual winrt::com_ptr<ITfCompositionSink> CreateCompositionSink(ITfContext *context) override {
        D(__FUNCTIONW__);
        return winrt::make<CompositionSink>(this, context);
    }

    virtual void OnCompositionTerminated(TfEditCookie ecWrite, ITfContext *context,
                                         ITfComposition *pComposition) override {
        D(__FUNCTIONW__);
        compositionMgr_->ClearComposition();
        candidateListUI_->Show(false);
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

        threadMgr_ = nullptr;
        clientId_ = TF_CLIENTID_NULL;
        activateFlags_ = 0;

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tid, DWORD dwFlags) override {
        D(__FUNCTIONW__, L" clientId: ", tid);
        TRY_FOR_HRESULT;

        threadMgr_.copy_from(pThreadMgr);
        clientId_ = tid;
        activateFlags_ = dwFlags;

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
        displayAttributes_.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) override {
        TRY_FOR_HRESULT;
        displayAttributes_->findByGuid(guid, ppInfo);
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
            openCloseCompartment_.GetValue(&val);

            if (val == false) {
                engine_->Reset();
                candidateListUI_->DestroyCandidateWindow();
            }
        }

        CATCH_FOR_HRESULT;
    }
};

//+---------------------------------------------------------------------------
//
// TextServiceFactory
//
//----------------------------------------------------------------------------

void TextServiceFactory::Create(TextService **ppService) {
    D(__FUNCTIONW__);
    as_self<TextService>(winrt::make_self<TextServiceImpl>()).copy_to(ppService);
}

} // namespace khiin::win32
