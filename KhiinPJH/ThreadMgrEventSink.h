#pragma once

#include "SinkManager.h"
#include "ITextService.h"

namespace Khiin {

struct ThreadMgrEventSink : winrt::implements<ThreadMgrEventSink, ITfThreadMgrEventSink> {
    ThreadMgrEventSink() = default;
    ~ThreadMgrEventSink();
    DELETE_COPY_AND_ASSIGN(ThreadMgrEventSink);

    HRESULT init(ITextService *pService);
    HRESULT uninit();

    // Inherited via implements
    virtual STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pdim) override;
    virtual STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pdim) override;
    virtual STDMETHODIMP OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus) override;
    virtual STDMETHODIMP OnPushContext(ITfContext *pic) override;
    virtual STDMETHODIMP OnPopContext(ITfContext *pic) override;

  private:
    winrt::com_ptr<ITextService> service = nullptr;
    SinkManager<ITfThreadMgrEventSink> threadMgrSinkMgr;
};

} // namespace Khiin
