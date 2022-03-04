#include "pch.h"

#include "Graphics.h"

namespace khiin::win32 {
using namespace winrt;

com_ptr<ID2D1Factory1> Graphics::CreateD2D1Factory() {
    auto factory = com_ptr<ID2D1Factory1>();
    auto hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.put());
    CHECK_HRESULT(hr);
    return factory;
}

com_ptr<IDWriteFactory3> Graphics::CreateDwriteFactory() {
    auto factory = com_ptr<IDWriteFactory3>();
    auto hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3),
                                    reinterpret_cast<IUnknown **>(factory.put()));
    CHECK_HRESULT(hr);
    return factory;
}

com_ptr<ID2D1HwndRenderTarget> Graphics::CreateRenderTarget(com_ptr<ID2D1Factory1> const &factory, HWND const &hwnd) {
    WINRT_ASSERT(factory);
    WINRT_ASSERT(hwnd);
    KHIIN_TRACE("");
    RECT rc;
    ::GetClientRect(hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
    auto target = com_ptr<ID2D1HwndRenderTarget>();

    auto props = D2D1::RenderTargetProperties();
    auto hwnd_props = D2D1::HwndRenderTargetProperties(hwnd, size);
    auto hr = factory->CreateHwndRenderTarget(props, hwnd_props, target.put());
    CHECK_HRESULT(hr);

    return target;
}

com_ptr<ID2D1DCRenderTarget> Graphics::CreateDCRenderTarget(com_ptr<ID2D1Factory1> const& factory) {
    WINRT_ASSERT(factory);
    KHIIN_TRACE("");
    auto props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                               D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 0,
                                               0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT);
    auto target = com_ptr<ID2D1DCRenderTarget>();
    auto hr = factory->CreateDCRenderTarget(&props, target.put());
    CHECK_HRESULT(hr);
    return target;
}

} // namespace khiin::win32