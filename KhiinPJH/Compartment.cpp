#include "pch.h"

#include "Compartment.h"

namespace khiin::win32 {

using namespace winrt;

void Compartment::Initialize(TfClientId clientId, IUnknown *compartment_provider, const GUID &guid, bool global) {
    WINRT_ASSERT(compartment_provider != nullptr);
    auto compartmentMgr = com_ptr<ITfCompartmentMgr>();

    if (global) {
        auto threadMgr = com_ptr<ITfThreadMgr>();
        winrt::check_hresult(compartment_provider->QueryInterface(threadMgr.put()));
        winrt::check_hresult(threadMgr->GetGlobalCompartment(compartmentMgr.put()));
    } else {
        winrt::check_hresult(compartment_provider->QueryInterface(compartmentMgr.put()));
    }

    winrt::check_hresult(compartmentMgr->GetCompartment(guid, compartment.put()));
    this->clientId = clientId;
    this->guid = guid;
}

void Compartment::Uninitialize() {
    compartment = nullptr;
    clientId = TF_CLIENTID_NULL;
    guid = GUID{};
}

void Compartment::GetValue(_Out_ DWORD *val) {
    VARIANT var;
    winrt::check_hresult(compartment->GetValue(&var));

    if (var.vt != VT_I4) {
        throw winrt::hresult_invalid_argument();
    }

    *val = var.lVal;
}

void Compartment::SetValue(_In_ const DWORD &val) {
    VARIANT var;
    ::VariantInit(&var);
    var.vt = VT_I4;
    var.lVal = val;
    winrt::check_hresult(compartment->SetValue(clientId, &var));
}

ITfCompartment* Compartment::getCompartment() {
    WINRT_ASSERT(compartment != nullptr);
    return compartment.get();
}

} // namespace khiin::win32
