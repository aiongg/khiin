#include "pch.h"

#include "KhiinClassFactory.h"

#include "TextService.h"
#include "DllModule.h"

namespace Khiin {

//+---------------------------------------------------------------------------
//
// IClassFactory
//
//----------------------------------------------------------------------------

STDMETHODIMP KhiinClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject) noexcept {
    auto hr = E_FAIL;

    if (ppvObject == nullptr) {
        return E_INVALIDARG;
    }
    *ppvObject = nullptr;
    if (pUnkOuter != nullptr) {
        return CLASS_E_NOAGGREGATION;
    }

    auto riidStr = std::wstring(39, '?');
    hr = ::StringFromGUID2(riid, &riidStr[0], 39);
    CHECK_RETURN_HRESULT(hr);

    D(L"QueryInterface: ", riidStr.c_str());

    auto textService = winrt::com_ptr<TextService>();
    hr = TextServiceFactory::create(textService.put());
    CHECK_RETURN_HRESULT(hr);

    hr = textService->QueryInterface(riid, ppvObject);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

STDMETHODIMP KhiinClassFactory::LockServer(BOOL fLock) {
    if (fLock) {
        DllModule::AddRef();
    } else {
        DllModule::Release();
    }

    return S_OK;
}

} // namespace Khiin
