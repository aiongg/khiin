#pragma once

#include "CompositionMgr.h"

namespace Khiin {

struct KeyEventSink : winrt::implements<KeyEventSink, ITfKeyEventSink> {
    KeyEventSink() = default;
    KeyEventSink(const KeyEventSink &) = delete;
    KeyEventSink &operator=(const KeyEventSink &) = delete;
    ~KeyEventSink() = default;

    HRESULT init(_In_ TfClientId clientId, _In_ ITfThreadMgr *threadMgr, _In_ CompositionMgr *compositionMgr);
    HRESULT uninit();

    HRESULT beginEditSession(ITfContext *ctx, WPARAM wParam, LPARAM lParam);

    // ITfKeyEventSink
    virtual STDMETHODIMP OnSetFocus(BOOL fForeground) override;
    virtual STDMETHODIMP OnTestKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten) override;

  private:
    TfClientId clientId = TF_CLIENTID_NULL;
    winrt::com_ptr<ITfThreadMgr> threadMgr = nullptr;
    winrt::com_ptr<ITfKeystrokeMgr> keystrokeMgr = nullptr;
    winrt::com_ptr<CompositionMgr> compositionMgr = nullptr;
};

} // namespace Khiin
