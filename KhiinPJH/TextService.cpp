#include "pch.h"

#include "TextService.h"

#include "CandidateListUI.h"
#include "Compartment.h"
#include "CompositionMgr.h"
#include "DisplayAttributeInfoEnum.h"
#include "KeyEventSink.h"
#include "TextEditSink.h"
#include "TextEngine.h"
#include "ThreadMgrEventSink.h"
#include "common.h"

namespace Khiin {

struct TextServiceImpl :
    winrt::implements<TextServiceImpl, ITfTextInputProcessorEx, ITfDisplayAttributeProvider, ITfThreadFocusSink,
                      ITfTextLayoutSink, ITfCompartmentEventSink, TextService> {
    TextServiceImpl() {
        compositionMgr = winrt::make_self<CompositionMgr>();
        threadMgrEventSink = winrt::make_self<ThreadMgrEventSink>();
        engine = winrt::make_self<TextEngine>();
        candidateListUI = winrt::make_self<CandidateListUI>();
        keyEventSink = winrt::make_self<KeyEventSink>();
    }

    HRESULT getTopContext(_Out_ ITfContext **ppContext) {
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

    ITfCompositionSink *getCompositionMgr() {
        return compositionMgr.as<ITfCompositionSink>().get();
    }

    TfClientId clientId() {
        return clientId_;
    }

    ITfThreadMgr *threadMgr() {
        return threadMgr_.get();
    }

    DWORD activateFlags() {
        return activateFlags_;
    }

  private:
    HRESULT onActivate() {
        auto hr = E_FAIL;

        auto pTextService = cast_as<TextService>(this);

        DisplayAttributeInfoEnum::load(displayAttributes.put());

        hr = compositionMgr->init(pTextService);
        CHECK_RETURN_HRESULT(hr);

        hr = threadMgrEventSink->init(pTextService);
        CHECK_RETURN_HRESULT(hr);

        hr = engine->init();
        CHECK_RETURN_HRESULT(hr);

        hr = candidateListUI->init(pTextService);
        CHECK_RETURN_HRESULT(hr);

        hr = keyEventSink->init(pTextService, candidateListUI.get(), engine.get());
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment.init(clientId_, threadMgr_.get(), GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
        CHECK_RETURN_HRESULT(hr);

        hr = keyboardDisabledCompartment.init(clientId_, threadMgr_.get(), GUID_COMPARTMENT_KEYBOARD_DISABLED);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseSinkInstaller.install(openCloseCompartment.getCompartment(), this);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment.set(true);
        CHECK_RETURN_HRESULT(hr);

        return S_OK;
    }

    HRESULT onDeactivate() {
        auto hr = E_FAIL;

        hr = engine->uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = candidateListUI->uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseSinkInstaller.uninstall();
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment.set(false);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment.uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = keyboardDisabledCompartment.uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = keyEventSink->uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = threadMgrEventSink->uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = compositionMgr->uninit();
        CHECK_RETURN_HRESULT(hr);

        displayAttributes = nullptr;

        return S_OK;
    }

    winrt::com_ptr<ITfThreadMgr> threadMgr_ = nullptr;
    TfClientId clientId_ = TF_CLIENTID_NULL;
    DWORD activateFlags_ = 0;
    Compartment openCloseCompartment;
    Compartment keyboardDisabledCompartment;
    DWORD openCloseSinkCookie = TF_INVALID_COOKIE;
    SinkManager<ITfCompartmentEventSink> openCloseSinkInstaller;

    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<DisplayAttributeInfoEnum> displayAttributes = nullptr;
    winrt::com_ptr<ThreadMgrEventSink> threadMgrEventSink = nullptr;
    winrt::com_ptr<KeyEventSink> keyEventSink = nullptr;
    winrt::com_ptr<CompositionMgr> compositionMgr = nullptr;
    winrt::com_ptr<CandidateListUI> candidateListUI = nullptr;
    winrt::com_ptr<TextEngine> engine = nullptr;

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
        displayAttributes.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
        return S_OK;
    }

    virtual STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) override {
        auto hr = displayAttributes->findByGuid(guid, ppInfo);
        CHECK_RETURN_HRESULT(hr);
        return S_OK;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCompartmentEventSink
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP OnChange(REFGUID rguid) {
        D(__FUNCTIONW__);
        auto hr = E_FAIL;

        if (rguid == GUID_COMPARTMENT_KEYBOARD_OPENCLOSE) {
            DWORD val;
            hr = openCloseCompartment.get(&val);
            CHECK_RETURN_HRESULT(hr);

            if (val == false) {
                engine->clear();
            }
        }

        return S_OK;
    }
};

//+---------------------------------------------------------------------------
//
// TextService
//
//----------------------------------------------------------------------------
auto static textServiceImpl = winrt::make_self<TextServiceImpl>();

TfClientId TextService::clientId() {
    return textServiceImpl->clientId();
}

ITfThreadMgr *TextService::threadMgr() {
    return textServiceImpl->threadMgr();
}

DWORD TextService::getActivateFlags() {
    return textServiceImpl->activateFlags();
}

IEnumTfDisplayAttributeInfo *TextService::displayAttrInfoEnum() {
    IEnumTfDisplayAttributeInfo *tmp = nullptr;
    textServiceImpl->EnumDisplayAttributeInfo(&tmp);
    return tmp;
}

ITfCompositionSink *TextService::compositionMgr() {
    return textServiceImpl->getCompositionMgr();
}

TextService *TextServiceFactory::create() {
    return cast_as<TextService>(textServiceImpl.get());
}

} // namespace Khiin
