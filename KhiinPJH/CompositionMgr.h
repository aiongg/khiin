#pragma once

namespace Khiin {

struct CompositionMgr : winrt::implements<CompositionMgr, ITfCompositionSink> {

    CompositionMgr() = default;
    CompositionMgr(const CompositionMgr &) = delete;
    CompositionMgr &operator=(const CompositionMgr &) = delete;
    ~CompositionMgr() = default;

    HRESULT init(TfClientId clientId);
    HRESULT uninit();

    bool composing();

    HRESULT startComposition(ITfContext *pContext);
    HRESULT doComposition(ITfContext *pContext, std::string text);
    HRESULT endComposition();

    // ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(TfEditCookie cookie, ITfComposition *pComposition) override;

  private:
    HRESULT startComposition(TfEditCookie cookie, ITfContext *pContext);
    HRESULT doComposition(TfEditCookie cookie, ITfContext *pContext, std::string text);
    HRESULT endComposition(TfEditCookie cookie);

    TfClientId clientId = TF_CLIENTID_NULL;
    winrt::com_ptr<ITfComposition> composition = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
};

} // namespace Khiin
