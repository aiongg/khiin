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
    HRESULT setText(TfEditCookie cookie, ITfContext *pContext, std::string text);
    HRESULT requestEditSession(ITfContext *pContext);
    HRESULT startComposition(TfEditCookie cookie, ITfContext *pContext);

    // ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(TfEditCookie cookie, ITfComposition *pComposition) override;

  private:
    TfClientId clientId = TF_CLIENTID_NULL;
    winrt::com_ptr<ITfComposition> composition = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
};

} // namespace Khiin
