#pragma once

namespace khiin::win32::tip {

class Compartment {
  public:
    void Initialize(TfClientId client_id, IUnknown *pUnknown, const GUID &guid, bool global = false);
    void Uninitialize();

    void GetValue(_Out_ DWORD *val);
    void SetValue(_In_ const DWORD &val);
    ITfCompartment *get();

  private:
    winrt::com_ptr<ITfCompartment> compartment;
    TfClientId client_id = TF_CLIENTID_NULL;
    GUID guid{};
};

} // namespace khiin::win32
