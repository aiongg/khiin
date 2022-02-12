#include "pch.h"

#include "Graphics.h"

namespace khiin::win32 {
using namespace winrt;

com_ptr<ID2D1Factory1> Graphics::CreateD2D1Factory() {
    auto factory = com_ptr<ID2D1Factory1>();
    check_hresult(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.put()));
    return factory;
}

com_ptr<IDWriteFactory3> Graphics::CreateDwriteFactory() {
    auto factory = com_ptr<IDWriteFactory3>();
    check_hresult(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3),
                                        reinterpret_cast<IUnknown **>(factory.put())));
    return factory;
}

com_ptr<ID2D1HwndRenderTarget> Graphics::CreateRenderTarget(com_ptr<ID2D1Factory1> const &factory, HWND const &hwnd) {
    WINRT_ASSERT(factory);
    RECT rc;
    ::GetClientRect(hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
    auto target = com_ptr<ID2D1HwndRenderTarget>();
    check_hresult(factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                  D2D1::HwndRenderTargetProperties(hwnd, size), target.put()));
    return target;
}

} // namespace khiin::win32