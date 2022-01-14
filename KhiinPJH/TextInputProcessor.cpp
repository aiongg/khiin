#include "pch.h"

#include "TextInputProcessor.h"

#include "Compartment.h"
#include "TextEditSink.h"

namespace Khiin {

//+---------------------------------------------------------------------------
//
// Current context getter
//
//----------------------------------------------------------------------------

HRESULT TextInputProcessor::getTopContext(_Out_ ITfContext **ppContext) {
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

//+---------------------------------------------------------------------------
//
// ITfTextInputProcessorEx
//
//----------------------------------------------------------------------------

STDMETHODIMP TextInputProcessor::Activate(ITfThreadMgr *ptim, TfClientId tid) {
    D(L"Activate");
    return ActivateEx(ptim, tid, 0);
}

STDMETHODIMP TextInputProcessor::Deactivate(void) {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;

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

    threadMgr = nullptr;
    clientId = TF_CLIENTID_NULL;
    dwActivateFlags = 0;

    return S_OK;
}

STDMETHODIMP TextInputProcessor::ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tid, DWORD dwFlags) {
    D(__FUNCTIONW__, L" clientId: ", tid);
    auto hr = E_FAIL;
    threadMgr.copy_from(pThreadMgr);
    clientId = tid;
    dwActivateFlags = dwFlags;

    compositionMgr = winrt::make_self<CompositionMgr>();
    hr = compositionMgr->init(clientId);
    CHECK_RETURN_HRESULT(hr);

    threadMgrEventSink = winrt::make_self<ThreadMgrEventSink>();
    hr = threadMgrEventSink->init(pThreadMgr);
    CHECK_RETURN_HRESULT(hr);
    
    keyEventSink = winrt::make_self<KeyEventSink>();
    hr = keyEventSink->init(clientId, pThreadMgr, compositionMgr.get());
    CHECK_RETURN_HRESULT(hr);
    
    hr = openCloseCompartment.init(clientId, pThreadMgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    CHECK_RETURN_HRESULT(hr);
    
    hr = keyboardDisabledCompartment.init(clientId, pThreadMgr, GUID_COMPARTMENT_KEYBOARD_DISABLED);
    CHECK_RETURN_HRESULT(hr);

    openCloseSinkInstaller.install(openCloseCompartment.getCompartment(), this);
    openCloseCompartment.set(true);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfDisplayAttributeProvider
//
//----------------------------------------------------------------------------

STDMETHODIMP TextInputProcessor::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}
STDMETHODIMP TextInputProcessor::GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadFocusSink
//
//----------------------------------------------------------------------------

STDMETHODIMP TextInputProcessor::OnSetThreadFocus(void) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}
STDMETHODIMP TextInputProcessor::OnKillThreadFocus(void) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfTextLayoutSink
//
//----------------------------------------------------------------------------

STDMETHODIMP TextInputProcessor::OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pView) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfCompartmentEventSink
//
//----------------------------------------------------------------------------

STDMETHODIMP TextInputProcessor::OnChange(REFGUID rguid) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

} // namespace Khiin
