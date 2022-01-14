#pragma once

namespace Khiin {

class Compartment {
  public:
    HRESULT init(TfClientId clientId, IUnknown *pUnknown, const GUID &guid, bool global = false);
    HRESULT uninit();

    HRESULT get(_Out_ DWORD *val);
    HRESULT set(_In_ const DWORD &val);
    ITfCompartment *getCompartment();

  private:
    winrt::com_ptr<ITfCompartment> compartment;
    TfClientId clientId = TF_CLIENTID_NULL;
    GUID guid{};
};

} // namespace Khiin
