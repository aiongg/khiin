#include "pch.h"

#include "ThreadMgrEventSink.h"

#include "CandidateListUI.h"
#include "EditSession.h"
#include "SinkManager.h"
#include "TextService.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;

struct ThreadMgrEventSinkImpl : implements<ThreadMgrEventSinkImpl, ITfThreadMgrEventSink, ThreadMgrEventSink> {
  private:
    void Initialize(TextService *service) override {
        KHIIN_TRACE("");
        auto hr = E_FAIL;
        m_service.copy_from(service);
        m_sinkmgr.Advise(service->thread_mgr().get(), this);
    }

    void Uninitialize() override {
        KHIIN_TRACE("");
        auto hr = E_FAIL;
        m_sinkmgr.Unadvise();
        m_service = nullptr;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfThreadMgrEventSink
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pdim) override {
        KHIIN_TRACE("");
        return S_OK;
    }

    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pdim) override {
        KHIIN_TRACE("");
        return S_OK;
    }

    STDMETHODIMP OnSetFocus(ITfDocumentMgr *docmgr_focus, ITfDocumentMgr *prev_docmgr_focus) override {
        TRY_FOR_HRESULT;

        //EditSession::HandleFocusChange(m_service.get(), docmgr_focus);

        auto candidate_context = m_service->candidate_ui()->context();
        if (candidate_context) {
            auto candidate_docmgr = com_ptr<ITfDocumentMgr>();
            check_hresult(candidate_context->GetDocumentMgr(candidate_docmgr.put()));
            if (candidate_docmgr.get() == docmgr_focus) {
                m_service->candidate_ui()->OnSetThreadFocus();
            } else {
                m_service->candidate_ui()->OnKillThreadFocus();
            }
        }

        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP OnPushContext(ITfContext *pic) override {
        KHIIN_TRACE("");
        return S_OK;
    }

    STDMETHODIMP OnPopContext(ITfContext *pic) override {
        KHIIN_TRACE("");
        return S_OK;
    }

    winrt::com_ptr<TextService> m_service = nullptr;
    SinkManager<ITfThreadMgrEventSink> m_sinkmgr;
};

} // namespace

ThreadMgrEventSink::~ThreadMgrEventSink() = default;

winrt::com_ptr<ThreadMgrEventSink> ThreadMgrEventSink::Create() {
    return as_self<ThreadMgrEventSink>(make_self<ThreadMgrEventSinkImpl>());
}

} // namespace khiin::win32::tip