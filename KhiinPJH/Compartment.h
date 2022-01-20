#pragma once

namespace Khiin {

class Compartment {
  public:
    void Initialize(TfClientId clientId, IUnknown *pUnknown, const GUID &guid, bool global = false);
    void Uninitialize();

    void GetValue(_Out_ DWORD *val);
    void SetValue(_In_ const DWORD &val);
    ITfCompartment *getCompartment();

  private:
    winrt::com_ptr<ITfCompartment> compartment;
    TfClientId clientId = TF_CLIENTID_NULL;
    GUID guid{};
};

} // namespace Khiin
