#include "pch.h"

#include "TextService.h"

#include "CandidateListUI.h"
#include "Compartment.h"
#include "DisplayAttributeInfoEnum.h"
#include "TextEditSink.h"

namespace Khiin {

HRESULT TextService::onStart() {
    auto hr = E_FAIL;

    DisplayAttributeInfoEnum::load(displayAttributes.put());

    compositionMgr = winrt::make_self<CompositionMgr>();
    hr = compositionMgr->init(clientId, displayAttributes.get());
    CHECK_RETURN_HRESULT(hr);

    threadMgrEventSink = winrt::make_self<ThreadMgrEventSink>();
    hr = threadMgrEventSink->init(threadMgr.get());
    CHECK_RETURN_HRESULT(hr);

    engine = winrt::make_self<TextEngine>();
    candidateListUI = winrt::make_self<CandidateListUI>();
    hr = candidateListUI->init(threadMgr.get());

    keyEventSink = winrt::make_self<KeyEventSink>();
    hr = keyEventSink->init(clientId, threadMgr.get(), compositionMgr.get(), candidateListUI.get(), engine.get());
    CHECK_RETURN_HRESULT(hr);

    hr = openCloseCompartment.init(clientId, threadMgr.get(), GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    CHECK_RETURN_HRESULT(hr);

    hr = keyboardDisabledCompartment.init(clientId, threadMgr.get(), GUID_COMPARTMENT_KEYBOARD_DISABLED);
    CHECK_RETURN_HRESULT(hr);

    hr = openCloseSinkInstaller.install(openCloseCompartment.getCompartment(), this);
    CHECK_RETURN_HRESULT(hr);

    hr = openCloseCompartment.set(true);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT TextService::onStop() {
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
    threadMgr = nullptr;
    clientId = TF_CLIENTID_NULL;
    dwActivateFlags = 0;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Current context getter
//
//----------------------------------------------------------------------------

HRESULT TextService::getTopContext(_Out_ ITfContext **ppContext) {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;

    auto documentMgr = winrt::com_ptr<ITfDocumentMgr>();
    hr = threadMgr->GetFocus(documentMgr.put());
    CHECK_RETURN_HRESULT(hr);

    auto context = winrt::com_ptr<ITfContext>();
    hr = documentMgr->GetTop(context.put());
    CHECK_RETURN_HRESULT(hr);

    context.copy_to(ppContext);

    return S_OK;
}

HRESULT TextService::setOpenClose(bool openclose) {
    return E_NOTIMPL;
}

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
    auto hr = onStop();
    CHECK_HRESULT(hr);
    return S_OK;
}

STDMETHODIMP TextService::ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tid, DWORD dwFlags) {
    D(__FUNCTIONW__, L" clientId: ", tid);

    threadMgr.copy_from(pThreadMgr);
    clientId = tid;
    dwActivateFlags = dwFlags;

    auto hr = onStart();
    CHECK_HRESULT(hr);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfDisplayAttributeProvider
//
//----------------------------------------------------------------------------

STDMETHODIMP TextService::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum) {
    D(__FUNCTIONW__);
    displayAttributes.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
    return S_OK;
}
STDMETHODIMP TextService::GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) {
    D(__FUNCTIONW__);
    auto hr = displayAttributes->findByGuid(guid, ppInfo);
    CHECK_RETURN_HRESULT(hr);
    return S_OK;
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
    auto hr = E_FAIL;

    if (rguid == GUID_COMPARTMENT_KEYBOARD_OPENCLOSE) {
        DWORD val;
        hr = openCloseCompartment.get(&val);
        CHECK_RETURN_HRESULT(hr);

        if (val == false) {
            engine->clear();
        }
    }
    return E_NOTIMPL;
}

} // namespace Khiin
