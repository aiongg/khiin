#include "pch.h"

#include "Compartment.h"

namespace Khiin {

using namespace winrt;

HRESULT Compartment::init(TfClientId clientId, IUnknown *pUnknown, const GUID &guid, bool global) {
    WINRT_ASSERT(pUnknown != nullptr);
    auto hr = E_FAIL;

    auto compartmentMgr = com_ptr<ITfCompartmentMgr>();

    if (global) {
        auto threadMgr = com_ptr<ITfThreadMgr>();
        hr = pUnknown->QueryInterface(threadMgr.put());
        CHECK_RETURN_HRESULT(hr);

        hr = threadMgr->GetGlobalCompartment(compartmentMgr.put());
        CHECK_RETURN_HRESULT(hr);
    } else {
        hr = pUnknown->QueryInterface(compartmentMgr.put());
        CHECK_RETURN_HRESULT(hr);
    }

    hr = compartmentMgr->GetCompartment(guid, compartment.put());
    CHECK_RETURN_HRESULT(hr);

    this->clientId = clientId;
    this->guid = guid;

    return S_OK;
}

HRESULT Compartment::uninit() {
    compartment = nullptr;
    clientId = TF_CLIENTID_NULL;
    guid = GUID{};
    return S_OK;
}

HRESULT Compartment::get(_Out_ DWORD *val) {
    VARIANT var;
    auto hr = compartment->GetValue(&var);
    CHECK_RETURN_HRESULT(hr);

    if (var.vt != VT_I4) {
        return E_FAIL;
    }

    *val = var.lVal;
    return S_OK;
}

HRESULT Compartment::set(_In_ const DWORD &val) {
    VARIANT var;
    ::VariantInit(&var);
    var.vt = VT_I4;
    var.lVal = val;
    auto hr = compartment->SetValue(clientId, &var);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

ITfCompartment* Compartment::getCompartment() {
    WINRT_ASSERT(compartment != nullptr);
    return compartment.get();
}

} // namespace Khiin
