#include "pch.h"

#include "ThreadMgrEventSink.h"

namespace Khiin {

ThreadMgrEventSink::ThreadMgrEventSink(TextService *pTextService) {
    service.copy_from(pTextService);
}

HRESULT ThreadMgrEventSink::init() {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    hr = threadMgrSinkMgr.install(service->threadMgr(), this);
    CHECK_RETURN_HRESULT(hr);
    return hr;
}

HRESULT ThreadMgrEventSink::uninit() {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    hr = threadMgrSinkMgr.uninstall();
    CHECK_RETURN_HRESULT(hr);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink
//
//----------------------------------------------------------------------------

STDMETHODIMP ThreadMgrEventSink::OnInitDocumentMgr(ITfDocumentMgr *pdim) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

STDMETHODIMP ThreadMgrEventSink::OnUninitDocumentMgr(ITfDocumentMgr *pdim) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

STDMETHODIMP ThreadMgrEventSink::OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

STDMETHODIMP ThreadMgrEventSink::OnPushContext(ITfContext *pic) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

STDMETHODIMP ThreadMgrEventSink::OnPopContext(ITfContext *pic) {
    D(__FUNCTIONW__);
    return E_NOTIMPL;
}

} // namespace Khiin