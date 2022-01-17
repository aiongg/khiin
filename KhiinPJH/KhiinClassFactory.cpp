#include "pch.h"

#include "KhiinClassFactory.h"

#include "TextService.h"

namespace Khiin {

//+---------------------------------------------------------------------------
//
// IClassFactory
//
//----------------------------------------------------------------------------

STDMETHODIMP KhiinClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject) noexcept {
    auto hr = E_FAIL;
    auto riidStr = std::wstring(39, '?');
    hr = ::StringFromGUID2(riid, &riidStr[0], 39);
    CHECK_RETURN_HRESULT(hr);

    D(L"QueryInterface: ", riidStr.c_str());
    auto textService = TextServiceFactory::create();
    hr = textService->QueryInterface(riid, ppvObject);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

STDMETHODIMP KhiinClassFactory::LockServer(BOOL fLock) {
    if (fLock) {
        KhiinClassFactory::AddRef();
    } else {
        KhiinClassFactory::Release();
    }

    return S_OK;
}

} // namespace Khiin
