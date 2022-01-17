#pragma once

namespace Khiin {

struct EditSession : winrt::implements<EditSession, ITfEditSession> {
    using CallbackFn = std::function<HRESULT(TfEditCookie cookie)>;

    static HRESULT request(TfClientId clientId, ITfContext *pContext, CallbackFn callback);

    // ITfEditSession
    virtual STDMETHODIMP DoEditSession(TfEditCookie cookie);

  private:
    HRESULT init(CallbackFn callback);
    HRESULT uninit();
    CallbackFn callback;

    DEFAULT_CTOR_DTOR(EditSession);
    DELETE_COPY_AND_ASSIGN(EditSession);
};

} // namespace Khiin
