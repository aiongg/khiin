#pragma once

namespace Khiin {

struct __declspec(uuid("829893f6-728d-11ec-8c6e-e0d46491b35a")) KhiinClassFactory
    : winrt::implements<KhiinClassFactory, IClassFactory> {
    // Inherited via implements
    STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid,
                                     void **ppvObject) noexcept override;

    STDMETHODIMP LockServer(BOOL fLock) override;
};

} // namespace Khiin
