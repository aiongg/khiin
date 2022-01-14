#include "pch.h"

#include "ThreadMgrEventSink.h"

namespace Khiin {

HRESULT ThreadMgrEventSink::init(ITfThreadMgr *threadMgr) {
    D(__FUNCTIONW__);
    WINRT_ASSERT(threadMgr != nullptr);

    this->threadMgr.copy_from(threadMgr);

    auto hr = E_FAIL;
    hr = threadMgrSinkMgr.install(threadMgr, this);
    CHECK_RETURN_HRESULT(hr);
    return hr;
}

HRESULT ThreadMgrEventSink::uninit() {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    hr = threadMgrSinkMgr.uninstall();
    CHECK_RETURN_HRESULT(hr);
    threadMgr = nullptr;
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