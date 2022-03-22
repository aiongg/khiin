#include "pch.h"

#include "ThreadMgrEventSink.h"

#include "CandidateListUI.h"
#include "CompositionMgr.h"
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

    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *docmgr) override {
        KHIIN_TRACE("");

        if (docmgr) {
            auto context = com_ptr<ITfContext>();
            check_hresult(docmgr->GetTop(context.put()));
            m_service->OnContextChange(context.get());
        }

        return S_OK;
    }

    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pdim) override {
        KHIIN_TRACE("");
        return S_OK;
    }

    STDMETHODIMP OnSetFocus(ITfDocumentMgr *docmgr_focus, ITfDocumentMgr *prev_docmgr_focus) override {
        KHIIN_TRACE("");
        return S_OK;
    }

    STDMETHODIMP OnPushContext(ITfContext *context) override {
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