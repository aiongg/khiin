#pragma once

#include "SinkManager.h"

namespace Khiin {

struct ThreadMgrEventSink : winrt::implements<ThreadMgrEventSink, ITfThreadMgrEventSink> {
    HRESULT init(ITfThreadMgr *threadMgr);
    HRESULT uninit();

    // Inherited via implements
    virtual STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pdim) override;
    virtual STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pdim) override;
    virtual STDMETHODIMP OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus) override;
    virtual STDMETHODIMP OnPushContext(ITfContext *pic) override;
    virtual STDMETHODIMP OnPopContext(ITfContext *pic) override;

  private:
    winrt::com_ptr<ITfThreadMgr> threadMgr = nullptr;
    SinkManager<ITfThreadMgrEventSink> threadMgrSinkMgr;
};

} // namespace Khiin
