#pragma once

#include "SinkManager.h"
#include "TextService.h"

namespace Khiin {

struct ThreadMgrEventSink : winrt::implements<ThreadMgrEventSink, ITfThreadMgrEventSink> {
    ThreadMgrEventSink(TextService *pService);

    HRESULT init();
    HRESULT uninit();

    // Inherited via implements
    virtual STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pdim) override;
    virtual STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pdim) override;
    virtual STDMETHODIMP OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus) override;
    virtual STDMETHODIMP OnPushContext(ITfContext *pic) override;
    virtual STDMETHODIMP OnPopContext(ITfContext *pic) override;

  private:
    winrt::com_ptr<TextService> service = nullptr;
    SinkManager<ITfThreadMgrEventSink> threadMgrSinkMgr;

    DELETE_COPY_AND_ASSIGN(ThreadMgrEventSink);
};

} // namespace Khiin
