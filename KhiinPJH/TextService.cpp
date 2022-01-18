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

namespace Khiin {

struct TextServiceImpl :
    winrt::implements<TextServiceImpl, // clang-format off
                      ITfTextInputProcessorEx,
                      ITfDisplayAttributeProvider,
                      ITfThreadFocusSink,
                      ITfTextLayoutSink,
                      ITfCompartmentEventSink,
                      TextService> { // clang-format on
    TextServiceImpl() {
        TextEngineFactory::create(engine_.put());
        compositionMgr_ = winrt::make_self<CompositionMgr>();
        threadMgrEventSink_ = winrt::make_self<ThreadMgrEventSink>();
        candidateListUI_ = winrt::make_self<CandidateListUI>();
        keyEventSink_ = winrt::make_self<KeyEventSink>();
    }

  private:
    HRESULT onActivate() {
        auto hr = E_FAIL;

        auto pTextService = cast_as<TextService>(this);

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

        hr = openCloseCompartment_.set(true);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseSinkMgr_.install(openCloseCompartment_.getCompartment(), this);
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

        hr = keyboardDisabledCompartment_.uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment_.uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = candidateListUI_->uninit();
        CHECK_RETURN_HRESULT(hr);

        hr = keyEventSink_->uninit();
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

    virtual HRESULT categoryMgr(ITfCategoryMgr **ppCategoryMgr) override {
        D(__FUNCTIONW__);
        auto hr = E_FAIL;
        auto tmp = winrt::com_ptr<ITfCategoryMgr>();
        hr = ::CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, tmp.put_void());
        CHECK_RETURN_HRESULT(hr);
        tmp.copy_to(ppCategoryMgr);
        return S_OK;
    }

    virtual HRESULT compositionSink(ITfContext *context, ITfCompositionSink **ppCompositionSink) override {
        D(__FUNCTIONW__);
        auto sink = winrt::make<CompositionSink>(this, context);
        sink.copy_to(ppCompositionSink);
        return S_OK;
    }

    virtual HRESULT onCompositionTerminated(TfEditCookie ecWrite, ITfContext *context,
                                            ITfComposition *pComposition) override {
        D(__FUNCTIONW__);
        compositionMgr_->clearComposition();
        candidateListUI_->onCompositionTerminated();
        return S_OK;
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
        D(__FUNCTIONW__);
        displayAttributes_.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
        return S_OK;
    }

    virtual STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) override {
        D(__FUNCTIONW__);
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
                hr = engine_->clear();
                CHECK_RETURN_HRESULT(hr);

                hr = candidateListUI_->onCompositionTerminated();
                CHECK_RETURN_HRESULT(hr);
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

HRESULT TextServiceFactory::create(TextService **ppService) {
    D(__FUNCTIONW__);
    as_self<TextService>(winrt::make_self<TextServiceImpl>()).copy_to(ppService);
    return S_OK;
}

} // namespace Khiin
