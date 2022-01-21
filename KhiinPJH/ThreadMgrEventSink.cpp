#include "pch.h"

#include "ThreadMgrEventSink.h"

namespace khiin::win32 {

ThreadMgrEventSink::~ThreadMgrEventSink() {
    Uninitialize();
}

void ThreadMgrEventSink::Initialize(TextService *pTextService) {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    service.copy_from(pTextService);
    threadMgrSinkMgr.Advise(service->threadMgr(), this);
}

void ThreadMgrEventSink::Uninitialize() {
    D(__FUNCTIONW__);
    auto hr = E_FAIL;
    threadMgrSinkMgr.Unadvise();
    service = nullptr;
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

} // namespace khiin::win32