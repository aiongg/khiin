#include "pch.h"

#include "KhiinClassFactory.h"

#include "TextService.h"
#include "DllModule.h"

namespace khiin::win32::tip {

//+---------------------------------------------------------------------------
//
// IClassFactory
//
//----------------------------------------------------------------------------

STDMETHODIMP KhiinClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject) noexcept {
    TRY_FOR_HRESULT;

    if (ppvObject == nullptr) {
        throw winrt::hresult_invalid_argument();
    }
    *ppvObject = nullptr;
    if (pUnkOuter != nullptr) {
        throw winrt::hresult_error(CLASS_E_NOAGGREGATION);
    }

    auto riidStr = std::wstring(39, '?');
#pragma warning(push)
#pragma warning(disable: 6031)
    ::StringFromGUID2(riid, &riidStr[0], 39);
#pragma warning(pop)
    KHIIN_DEBUG(L"QI: {}", riidStr.c_str());

    auto tip = TextService::Create();
    winrt::check_hresult(tip->QueryInterface(riid, ppvObject));

    CATCH_FOR_HRESULT;
}

STDMETHODIMP KhiinClassFactory::LockServer(BOOL fLock) {
    if (fLock) {
        DllModule::AddRef();
    } else {
        DllModule::Release();
    }

    return S_OK;
}

} // namespace khiin::win32
