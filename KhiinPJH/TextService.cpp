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

namespace Khiin {

class TextServiceImpl {
  public:
    HRESULT Activate(TextService *pTextService) {
        auto hr = E_FAIL;

        textService.copy_from(pTextService);
        auto clientId = textService->clientId();
        auto pThreadMgr = textService->threadMgr();

        DisplayAttributeInfoEnum::load(displayAttributes.put());

        compositionMgr = winrt::make_self<CompositionMgr>(pTextService);
        hr = compositionMgr->init();
        CHECK_RETURN_HRESULT(hr);

        threadMgrEventSink = winrt::make_self<ThreadMgrEventSink>(pTextService);
        hr = threadMgrEventSink->init();
        CHECK_RETURN_HRESULT(hr);

        engine = winrt::make_self<TextEngine>();
        candidateListUI = winrt::make_self<CandidateListUI>();
        hr = candidateListUI->init(pThreadMgr);

        keyEventSink = winrt::make_self<KeyEventSink>();
        hr = keyEventSink->init(clientId, pThreadMgr, compositionMgr.get(), candidateListUI.get(), engine.get());
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment.init(clientId, pThreadMgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
        CHECK_RETURN_HRESULT(hr);

        hr = keyboardDisabledCompartment.init(clientId, pThreadMgr, GUID_COMPARTMENT_KEYBOARD_DISABLED);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseSinkInstaller.install(openCloseCompartment.getCompartment(), pTextService);
        CHECK_RETURN_HRESULT(hr);

        hr = openCloseCompartment.set(true);
        CHECK_RETURN_HRESULT(hr);

        return S_OK;
    }

    HRESULT Deactivate() {
        auto hr = E_FAIL;

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

    HRESULT EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum) {
        displayAttributes.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
        return S_OK;
    }

    HRESULT GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) {
        auto hr = displayAttributes->findByGuid(guid, ppInfo);
        CHECK_RETURN_HRESULT(hr);
        return S_OK;
    }

    HRESULT OnOpenCloseChange(REFGUID rguid) {
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

    HRESULT getTopContext(_Out_ ITfContext **ppContext) {
        D(__FUNCTIONW__);
        auto hr = E_FAIL;

        auto documentMgr = winrt::com_ptr<ITfDocumentMgr>();
        hr = textService->threadMgr()->GetFocus(documentMgr.put());
        CHECK_RETURN_HRESULT(hr);

        auto context = winrt::com_ptr<ITfContext>();
        hr = documentMgr->GetTop(context.put());
        CHECK_RETURN_HRESULT(hr);

        context.copy_to(ppContext);

        return S_OK;
    }

  private:
    Compartment openCloseCompartment;
    Compartment keyboardDisabledCompartment;
    DWORD openCloseSinkCookie = TF_INVALID_COOKIE;
    SinkManager<ITfCompartmentEventSink> openCloseSinkInstaller;

    winrt::com_ptr<TextService> textService = nullptr;
    winrt::com_ptr<DisplayAttributeInfoEnum> displayAttributes = nullptr;
    winrt::com_ptr<ThreadMgrEventSink> threadMgrEventSink = nullptr;
    winrt::com_ptr<KeyEventSink> keyEventSink = nullptr;
    winrt::com_ptr<CompositionMgr> compositionMgr = nullptr;
    winrt::com_ptr<CandidateListUI> candidateListUI = nullptr;
    winrt::com_ptr<TextEngine> engine = nullptr;
};

auto static textServiceImpl = TextServiceImpl();

//+---------------------------------------------------------------------------
//
// TextService
//
//----------------------------------------------------------------------------

TfClientId TextService::clientId() {
    return clientId_;
}

ITfThreadMgr *TextService::threadMgr() {
    return threadMgr_.get();
}

DWORD TextService::getActivateFlags() {
    return dwActivateFlags;
}

IEnumTfDisplayAttributeInfo *TextService::displayAttrInfoEnum() {
    IEnumTfDisplayAttributeInfo *tmp = nullptr;
    EnumDisplayAttributeInfo(&tmp);
    return tmp;
}

//+---------------------------------------------------------------------------
//
// Current context getter
//
//----------------------------------------------------------------------------

//+---------------------------------------------------------------------------
//
// ITfTextInputProcessorEx
//
//----------------------------------------------------------------------------

STDMETHODIMP TextService::Activate(ITfThreadMgr *ptim, TfClientId tid) {
    D(L"Activate");
    return ActivateEx(ptim, tid, 0);
}

STDMETHODIMP TextService::Deactivate(void) {
    D(__FUNCTIONW__);

    threadMgr_ = nullptr;
    clientId_ = TF_CLIENTID_NULL;
    dwActivateFlags = 0;

    return textServiceImpl.Deactivate();
}

STDMETHODIMP TextService::ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tid, DWORD dwFlags) {
    D(__FUNCTIONW__, L" clientId: ", tid);

    threadMgr_.copy_from(pThreadMgr);
    clientId_ = tid;
    dwActivateFlags = dwFlags;

    return textServiceImpl.Activate(this);
}

//+---------------------------------------------------------------------------
//
// ITfDisplayAttributeProvider
//
//----------------------------------------------------------------------------

STDMETHODIMP TextService::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum) {
    D(__FUNCTIONW__);
    return textServiceImpl.EnumDisplayAttributeInfo(ppEnum);
}

STDMETHODIMP TextService::GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) {
    D(__FUNCTIONW__);
    return textServiceImpl.GetDisplayAttributeInfo(guid, ppInfo);
}

//+---------------------------------------------------------------------------
//
// ITfThreadFocusSink
//
//----------------------------------------------------------------------------

STDMETHODIMP TextService::OnSetThreadFocus(void) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}
STDMETHODIMP TextService::OnKillThreadFocus(void) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfTextLayoutSink
//
//----------------------------------------------------------------------------

STDMETHODIMP TextService::OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pView) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfCompartmentEventSink
//
//----------------------------------------------------------------------------

STDMETHODIMP TextService::OnChange(REFGUID rguid) {
    D(__FUNCTIONW__);
    return textServiceImpl.OnOpenCloseChange(rguid);
}

} // namespace Khiin
