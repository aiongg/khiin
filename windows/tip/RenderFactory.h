#pragma once

namespace khiin::win32 {

struct RenderFactory : winrt::implements<RenderFactory, IUnknown> {
  public:
    static winrt::com_ptr<RenderFactory> Create();
    static void Shutdown();

    virtual winrt::com_ptr<ID2D1DCRenderTarget> CreateDCRenderTarget() = 0;
    virtual winrt::com_ptr<IDWriteTextFormat> CreateTextFormat(std::string const &font_name, float font_size) = 0;
    virtual winrt::com_ptr<IDWriteTextLayout> CreateTextLayout(std::string const &value,
                                                               winrt::com_ptr<IDWriteTextFormat> const &format,
                                                               uint32_t max_width, uint32_t max_height) = 0;
    virtual winrt::com_ptr<ID2D1Bitmap> CreateBitmap(winrt::com_ptr<ID2D1DCRenderTarget> const &target,
                                                     HICON hicon) = 0;
    virtual winrt::com_ptr<IDWriteFactory3> DWrite3() = 0;
};

} // namespace khiin::win32
