#include "pch.h"

#include "RenderFactory.h"
#include "Utils.h"

namespace khiin::win32 {
namespace {
using namespace winrt;

class RenderFactoryImpl : public RenderFactory {
  public:
    void Initialize() {
        CreateD2D1Factory();
        CreateDwriteFactory();
    }

    virtual com_ptr<ID2D1DCRenderTarget> CreateDCRenderTarget() override {
        auto props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                                  D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
                                                  0, 0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT);
        auto ret = com_ptr<ID2D1DCRenderTarget>();
        auto hr = m_d2d1->CreateDCRenderTarget(&props, ret.put());
        CHECK_HRESULT(hr);
        return ret;
    }

    virtual com_ptr<IDWriteTextFormat> CreateTextFormat(std::string const &font_name, float font_size) override {
        auto ret = com_ptr<IDWriteTextFormat>();
        auto wfont = Utils::Widen(font_name);
        check_hresult(m_dwrite->CreateTextFormat(wfont.c_str(), NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
                                                 DWRITE_FONT_STRETCH_NORMAL, font_size, L"en-us", ret.put()));
        check_hresult(ret->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
        check_hresult(ret->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
        return ret;
    }

    virtual com_ptr<IDWriteTextLayout> CreateTextLayout(std::string const &value, com_ptr<IDWriteTextFormat> const &format,
                                                               uint32_t max_width, uint32_t max_height) override {
        auto ret = com_ptr<IDWriteTextLayout>();
        auto wvalue = Utils::Widen(value);
        check_hresult(m_dwrite->CreateTextLayout(wvalue.c_str(), static_cast<UINT32>(wvalue.size() + 1),
                                                 format.get(), static_cast<float>(max_width),
                                                 static_cast<float>(max_height), ret.put()));
        return ret;
    }

  private:
    void CreateD2D1Factory() {
        auto hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2d1.put());
        CHECK_HRESULT(hr);
    }

    void CreateDwriteFactory() {
        auto hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3),
                                        reinterpret_cast<IUnknown **>(m_dwrite.put()));
        CHECK_HRESULT(hr);
    }

    com_ptr<ID2D1Factory1> m_d2d1 = nullptr;
    com_ptr<IDWriteFactory3> m_dwrite = nullptr;
};

} // namespace

std::unique_ptr<RenderFactory> RenderFactory::Create() {
    auto factory = new RenderFactoryImpl();
    factory->Initialize();
    auto ret = std::unique_ptr<RenderFactory>(factory);
    return ret;
}

} // namespace khiin::win32