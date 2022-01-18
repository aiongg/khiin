#include "pch.h"

#include "ThreadMgrEventSink.h"

namespace Khiin {

ThreadMgrEventSink::~ThreadMgrEventSink() {
    uninit();
}

HRESULT ThreadMgrEventSink::init(TextService *pTextService) {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    service.copy_from(pTextService);
    hr = threadMgrSinkMgr.install(service->threadMgr(), this);
    CHECK_RETURN_HRESULT(hr);
    return hr;
}

HRESULT ThreadMgrEventSink::uninit() {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    hr = threadMgrSinkMgr.uninstall();
    service = nullptr;
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
    return S_OK;
}

STDMETHODIMP ThreadMgrEventSink::OnUninitDocumentMgr(ITfDocumentMgr *pdim) {
    D(__FUNCTIONW__);
    return S_OK;
}

STDMETHODIMP ThreadMgrEventSink::OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus) {
    D(__FUNCTIONW__);
    return S_OK;
}

STDMETHODIMP ThreadMgrEventSink::OnPushContext(ITfContext *pic) {
    D(__FUNCTIONW__);
    return S_OK;
}

STDMETHODIMP ThreadMgrEventSink::OnPopContext(ITfContext *pic) {
    D(__FUNCTIONW__);
    return S_OK;
}

} // namespace Khiin