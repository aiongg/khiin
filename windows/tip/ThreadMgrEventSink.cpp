#include "pch.h"

#include "ThreadMgrEventSink.h"

#include "CandidateListUI.h"

namespace khiin::win32 {
using namespace winrt;

ThreadMgrEventSink::~ThreadMgrEventSink() {
    Uninitialize();
}

void ThreadMgrEventSink::Initialize(TextService *text_service) {
    KHIIN_TRACE("");
    auto hr = E_FAIL;
    service.copy_from(text_service);
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

STDMETHODIMP ThreadMgrEventSink::OnSetFocus(ITfDocumentMgr *docmgr_focus, ITfDocumentMgr *prev_docmgr_focus) {
    TRY_FOR_HRESULT;

    auto candidate_context = service->candidate_ui()->context();

    if (candidate_context) {
        auto candidate_docmgr = com_ptr<ITfDocumentMgr>();
        check_hresult(candidate_context->GetDocumentMgr(candidate_docmgr.put()));
        if (candidate_docmgr.get() == docmgr_focus) {
            service->candidate_ui()->OnSetThreadFocus();
        } else {
            service->candidate_ui()->OnKillThreadFocus();
        }
    }

    CATCH_FOR_HRESULT;
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