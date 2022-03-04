#include "pch.h"

#include "ThreadMgrEventSink.h"

namespace khiin::win32 {

ThreadMgrEventSink::~ThreadMgrEventSink() {
    Uninitialize();
}

void ThreadMgrEventSink::Initialize(TextService *pTextService) {
    KHIIN_TRACE("");
    auto hr = E_FAIL;
    service.copy_from(pTextService);
    threadMgrSinkMgr.Advise(service->thread_mgr(), this);
}

void ThreadMgrEventSink::Uninitialize() {
    KHIIN_TRACE("");
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
    KHIIN_TRACE("");
    return S_OK;
}

STDMETHODIMP ThreadMgrEventSink::OnUninitDocumentMgr(ITfDocumentMgr *pdim) {
    KHIIN_TRACE("");
    return S_OK;
}

STDMETHODIMP ThreadMgrEventSink::OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus) {
    KHIIN_TRACE("");
    return S_OK;
}

STDMETHODIMP ThreadMgrEventSink::OnPushContext(ITfContext *pic) {
    KHIIN_TRACE("");
    return S_OK;
}

STDMETHODIMP ThreadMgrEventSink::OnPopContext(ITfContext *pic) {
    KHIIN_TRACE("");
    return S_OK;
}

} // namespace khiin::win32