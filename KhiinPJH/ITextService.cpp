#include "pch.h"

#include "ITextService.h"

#include "CandidateListUI.h"
#include "Compartment.h"
#include "CompositionMgr.h"
#include "DisplayAttributeInfoEnum.h"
#include "KeyEventSink.h"
#include "TextEditSink.h"
#include "ThreadMgrEventSink.h"
#include "common.h"

namespace Khiin {

struct TextServiceImpl :
    winrt::implements<TextServiceImpl, // clang-format off
                      ITfTextInputProcessorEx,
                      ITfDisplayAttributeProvider,
                      ITfThreadFocusSink,
                      ITfTextLayoutSink,
                      ITfCompartmentEventSink,
                      ITextService> { // clang-format on
    TextServiceImpl() {
        TextEngineFactory::create(engine_.put());
        compositionMgr_ = winrt::make_self<CompositionMgr>();
        threadMgrEventSink_ = winrt::make_self<ThreadMgrEventSink>();
        candidateListUI_ = winrt::make_self<CandidateListUI>();
        keyEventSink_ = winrt::make_self<KeyEventSink>();
    }

    virtual TfClientId clientId() override {
        return clientId_;
    }

    virtual ITfThreadMgr *threadMgr() override {
        return threadMgr_.get();
    }

    virtual DWORD activateFlags() override {
        return activateFlags_;
    }

    virtual ITfCompositionSink *compositionMgr() override {
        return compositionMgr_.as<ITfCompositionSink>().get();
    }

    virtual IEnumTfDisplayAttributeInfo *displayAttrInfoEnum() override {
        IEnumTfDisplayAttributeInfo *tmp = nullptr;
        EnumDisplayAttributeInfo(&tmp);
        return tmp;
    }

    virtual ITextEngine *engine() override {
        return engine_.get();
    }

    virtual ITfUIElement *candidateUI() override {
        return candidateListUI_.as<ITfUIElement>().get();
    }

    virtual HRESULT topContext(_Out_ ITfContext **ppContext) override {
        D(__FUNCTIONW__);
        auto hr = E_FAIL;

        auto documentMgr = winrt::com_ptr<ITfDocumentMgr>();
        hr = threadMgr_->GetFocus(documentMgr.put());
        CHECK_RETURN_HRESULT(hr);

        auto context = winrt::com_ptr<ITfContext>();
        hr = documentMgr->GetTop(context.put());
        CHECK_RETURN_HRESULT(hr);

        context.copy_to(ppContext);

        return S_OK;
    }

  private:
    HRESULT onActivate() {
        auto hr = E_FAIL;

        auto pTextService = cast_as<ITextService>(this);

        DisplayAttributeInfoEnum::load(displayAttributes_.put());

        hr = compositionMgr_->init(pTextService);
        CHECK_RETURN_HRESULT(hr);

        hr = threadMgrEventSink_->init(pTextService);
        CHECK_RETURN_HRESULT(hr);

        hr = candidateListUI_->init(pTextService);
        CHECK_RETURN_HRESULT(hr);

        hr = keyEventSink_->init(pTextService);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment_.init(clientId_, threadMgr_.get(), GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
        CHECK_RETURN_HRESULT(hr);

        hr = keyboardDisabledCompartment_.init(clientId_, threadMgr_.get(), GUID_COMPARTMENT_KEYBOARD_DISABLED);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseSinkMgr_.install(openCloseCompartment_.getCompartment(), this);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment_.set(true);
        CHECK_RETURN_HRESULT(hr);

        hr = engine_->init();
        CHECK_RETURN_HRESULT(hr);

        return S_OK;
    }

    HRESULT onDeactivate() {
        auto hr = E_FAIL;

        hr = engine_->uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseSinkMgr_.uninstall();
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment_.set(false);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment_.uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = keyboardDisabledCompartment_.uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = keyEventSink_->uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = candidateListUI_->uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = threadMgrEventSink_->uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = compositionMgr_->uninit();
        CHECK_RETURN_HRESULT(hr);

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
    winrt::com_ptr<ITextEngine> engine_ = nullptr;

  public:
    //+---------------------------------------------------------------------------
    //
    // ITfTextInputProcessorEx
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid) override {
        D(L"Activate");
        return ActivateEx(ptim, tid, 0);
    }

    virtual STDMETHODIMP Deactivate(void) override {
        D(__FUNCTIONW__);

        auto hr = onDeactivate();
        CHECK_RETURN_HRESULT(hr);

        threadMgr_ = nullptr;
        clientId_ = TF_CLIENTID_NULL;
        activateFlags_ = 0;

        return S_OK;
    }

    virtual STDMETHODIMP ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tid, DWORD dwFlags) override {
        D(__FUNCTIONW__, L" clientId: ", tid);

        threadMgr_.copy_from(pThreadMgr);
        clientId_ = tid;
        activateFlags_ = dwFlags;

        return onActivate();
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
        displayAttributes_.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
        return S_OK;
    }

    virtual STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) override {
        auto hr = displayAttributes_->findByGuid(guid, ppInfo);
        CHECK_RETURN_HRESULT(hr);
        return S_OK;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCompartmentEventSink
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP OnChange(REFGUID rguid) override {
        D(__FUNCTIONW__);
        auto hr = E_FAIL;

        if (rguid == GUID_COMPARTMENT_KEYBOARD_OPENCLOSE) {
            DWORD val;
            hr = openCloseCompartment_.get(&val);
            CHECK_RETURN_HRESULT(hr);

            if (val == false) {
                engine_->clear();
            }
        }

        return S_OK;
    }
};

//+---------------------------------------------------------------------------
//
// TextServiceFactory::create
//
//----------------------------------------------------------------------------

HRESULT TextServiceFactory::create(ITextService **ppService) {
    as_self<ITextService>(winrt::make_self<TextServiceImpl>()).copy_to(ppService);
    return S_OK;
}

} // namespace Khiin
