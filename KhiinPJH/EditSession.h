#pragma once

namespace Khiin {

struct EditSession : winrt::implements<EditSession, ITfEditSession> {
    using CallbackFn = std::function<HRESULT(TfEditCookie cookie)>;

    EditSession() = default;
    EditSession(const EditSession &) = delete;
    EditSession &operator=(const EditSession &) = delete;
    ~EditSession() = default;

    static HRESULT request(TfClientId clientId, ITfContext *pContext, CallbackFn callback);

    // ITfEditSession
    virtual STDMETHODIMP DoEditSession(TfEditCookie cookie);

  private:
    HRESULT init(CallbackFn callback);
    HRESULT uninit();

    CallbackFn callback;
};

} // namespace Khiin
