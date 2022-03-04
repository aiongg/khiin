#pragma once

#include "SinkManager.h"
#include "TextService.h"

namespace khiin::win32 {

struct ThreadMgrEventSink : winrt::implements<ThreadMgrEventSink, ITfThreadMgrEventSink> {
    ThreadMgrEventSink() = default;
    ThreadMgrEventSink(const ThreadMgrEventSink &) = delete;
    ThreadMgrEventSink &operator=(const ThreadMgrEventSink &) = delete;
    ~ThreadMgrEventSink();

    void Initialize(TextService *pService);
    void Uninitialize();

    // Inherited via implements
    virtual STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pdim) override;
    virtual STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pdim) override;
    virtual STDMETHODIMP OnSetFocus(ITfDocumentMgr *docmgr_focus, ITfDocumentMgr *prev_docmgr_focus) override;
    virtual STDMETHODIMP OnPushContext(ITfContext *pic) override;
    virtual STDMETHODIMP OnPopContext(ITfContext *pic) override;

  private:
    winrt::com_ptr<TextService> service = nullptr;
    SinkManager<ITfThreadMgrEventSink> threadMgrSinkMgr;
};

} // namespace khiin::win32
