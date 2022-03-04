#pragma once

namespace khiin::win32 {

class Graphics {
  public:
    static winrt::com_ptr<ID2D1Factory1> CreateD2D1Factory();
    static winrt::com_ptr<IDWriteFactory3> CreateDwriteFactory();
    static winrt::com_ptr<ID2D1HwndRenderTarget> CreateRenderTarget(winrt::com_ptr<ID2D1Factory1> const &factory,
                                                                    HWND const &hwnd);
    static winrt::com_ptr<ID2D1DCRenderTarget> CreateDCRenderTarget(winrt::com_ptr<ID2D1Factory1> const &factory);
};

} // namespace khiin::win32
