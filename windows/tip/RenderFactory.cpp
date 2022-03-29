#include "pch.h"

#include "RenderFactory.h"

#include <filesystem>
#include <memory>

#include "FontLoader.h"
#include "Utils.h"
#include "common.h"

namespace khiin::win32 {
namespace {
using namespace winrt;

constexpr const wchar_t *kHanjiFontName = L"Source Han Serif TC";
constexpr const wchar_t *kLomajiFontName = L"Chuigoan Serif 4";

const std::vector<DWRITE_UNICODE_RANGE> kGlyphRanges = {
    {0x2E80, 0x2EFF},   // CJK Radicals Supplement
    {0x2F00, 0x2FDF},   // Kangxi Radicals
    {0x3000, 0x303F},   // CJK Symbols and Punctuation
    {0x31F0, 0x31FF},   // Katakana Phonetic Extensions
    {0x3220, 0x32FF},   // Enclosed CJK Letters and Months
    {0x3400, 0x4DBF},   // CJK Unified Ideographs Extension A
    {0x4E00, 0x62FF},   // CJK Unified Ideographs
    {0x6300, 0x77FF},   // CJK Unified Ideographs
    {0x7800, 0x8CFF},   // CJK Unified Ideographs
    {0x8D00, 0x9FFF},   // CJK Unified Ideographs
    {0xF900, 0xFAFF},   // CJK Compatibility Ideographs
    {0xFF00, 0xFFEF},   // Halfwidth and Fullwidth Forms
    {0x20000, 0x215FF}, // CJK Unified Ideographs Extension B
    {0x21600, 0x230FF}, // CJK Unified Ideographs Extension B
    {0x23100, 0x245FF}, // CJK Unified Ideographs Extension B
    {0x24600, 0x260FF}, // CJK Unified Ideographs Extension B
    {0x26100, 0x275FF}, // CJK Unified Ideographs Extension B
    {0x27600, 0x290FF}, // CJK Unified Ideographs Extension B
    {0x29100, 0x2A6DF}, // CJK Unified Ideographs Extension B
    {0x2A700, 0x2B73F}, // CJK Unified Ideographs Extension C
    {0x2B740, 0x2B81F}, // CJK Unified Ideographs Extension D
    {0x2B820, 0x2CEAF}, // CJK Unified Ideographs Extension E
    {0x2CEB0, 0x2EBEF}, // CJK Unified Ideographs Extension F
    {0x30000, 0x3134F}, // CJK Unified Ideographs Extension G
};

struct RenderFactoryImpl : implements<RenderFactoryImpl, RenderFactory> {
    void Initialize() {
        CreateD2D1Factory();
        CreateDwriteFactory();
        CreateWicFactory();
        // CreateFontCollection();
        CreateFallbackFonts();
    }

    void Uninitialize() {
        auto hr = m_dwrite->UnregisterFontCollectionLoader(m_loader.get());
        CHECK_HRESULT(hr);

        m_d2d1 = nullptr;
        m_dwrite = nullptr;
        m_wic = nullptr;
        m_fontcollection = nullptr;
        m_fallback = nullptr;

        // FontLoader::Shutdown();
    }

    com_ptr<IDWriteFactory3> DWrite3() {
        return m_dwrite;
    }

    com_ptr<ID2D1DCRenderTarget> CreateDCRenderTarget() override {
        auto props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                                  D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
                                                  0, 0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT);
        auto ret = com_ptr<ID2D1DCRenderTarget>();
        auto hr = m_d2d1->CreateDCRenderTarget(&props, ret.put());
        CHECK_HRESULT(hr);
        return ret;
    }

    com_ptr<IDWriteTextFormat> CreateTextFormat(std::string const &font_name, float font_size) override {
        auto ret = com_ptr<IDWriteTextFormat>();
        check_hresult(m_dwrite->CreateTextFormat(kLomajiFontName, m_fontcollection.get(), DWRITE_FONT_WEIGHT_REGULAR,
                                                 DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, font_size,
                                                 L"en-us", ret.put()));
        check_hresult(ret->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
        check_hresult(ret->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
        return ret;
    }

    com_ptr<IDWriteTextLayout> CreateTextLayout(std::string const &value, com_ptr<IDWriteTextFormat> const &format,
                                                uint32_t max_width, uint32_t max_height) override {

        auto ret = com_ptr<IDWriteTextLayout>();
        auto wvalue = Utils::Widen(value);
        check_hresult(m_dwrite->CreateTextLayout(wvalue.c_str(), static_cast<UINT32>(wvalue.size() + 1), format.get(),
                                                 static_cast<float>(max_width), static_cast<float>(max_height),
                                                 ret.put()));

        if (auto tmp = ret.try_as<IDWriteTextLayout2>()) {
            AddFallbacks(tmp.get());
        }

        return ret;
    }

    winrt::com_ptr<ID2D1Bitmap> CreateBitmap(com_ptr<ID2D1DCRenderTarget> const &target, HICON hicon) override {
        auto wic_bmp = com_ptr<IWICBitmap>();
        check_hresult(m_wic->CreateBitmapFromHICON(hicon, wic_bmp.put()));
        auto converter = com_ptr<IWICFormatConverter>();
        m_wic->CreateFormatConverter(converter.put());
        converter->Initialize(wic_bmp.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0,
                              WICBitmapPaletteTypeMedianCut);
        auto d2d1_bmp = com_ptr<ID2D1Bitmap>();
        target->CreateBitmapFromWicBitmap(converter.get(), d2d1_bmp.put());
        return d2d1_bmp;
    }

  private:
    void CreateD2D1Factory() {
        if (!m_d2d1) {
            auto hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2d1.put());
            CHECK_HRESULT(hr);
        }
    }

    void CreateDwriteFactory() {
        if (!m_dwrite) {
            auto hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3),
                                            reinterpret_cast<IUnknown **>(m_dwrite.put()));
            CHECK_HRESULT(hr);

            m_loader = FontLoader::Create();

            hr = m_dwrite->RegisterFontCollectionLoader(m_loader.get());
            CHECK_HRESULT(hr);

            hr = m_dwrite->CreateCustomFontCollection(m_loader.get(), FontLoader::collection_key,
                                                      static_cast<UINT32>(strlen(FontLoader::collection_key)),
                                                      m_fontcollection.put());
            CHECK_HRESULT(hr);
        }
    }

    void CreateWicFactory() {
        if (!m_wic) {
            check_hresult(
                ::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(m_wic.put())));
        }
    }

    void CreateFontCollection() {
        if (!m_fontcollection) {
            // com_ptr<FontLoader> loader = make_self<FontLoader>();
            // auto collection = com_ptr<IDWriteFontCollection>();
            // auto hr = m_dwrite->CreateCustomFontCollection(loader.get(), FontLoader::collection_key,
            //                                               static_cast<UINT32>(strlen(FontLoader::collection_key)),
            //                                               collection.put());
            // CHECK_HRESULT(hr);
            // FontLoader::AvailableFontFamilies();
            //}
        }
    }

    void CreateFallbackFonts() {
        if (!m_fallback) {
            auto fallback_builder = com_ptr<IDWriteFontFallbackBuilder>();
            m_dwrite->CreateFontFallbackBuilder(fallback_builder.put());

            auto families = std::vector<const wchar_t *>{kHanjiFontName};

            // DWRITE_UNICODE_RANGE test = {0x7121, 0x7122};
            // auto hr = fallback_builder->AddMapping(&test, 1, families.data(), families.size(),
            // m_fontcollection.get());
            auto ranges = kGlyphRanges.data();
            auto n_ranges = static_cast<UINT32>(kGlyphRanges.size());
            auto n_families = static_cast<UINT32>(families.size());
            auto hr =
                fallback_builder->AddMapping(ranges, n_ranges, families.data(), n_families, m_fontcollection.get());
            CHECK_HRESULT(hr);

            auto system_fallbacks = com_ptr<IDWriteFontFallback>();
            m_dwrite->GetSystemFontFallback(system_fallbacks.put());
            fallback_builder->AddMappings(system_fallbacks.get());
            fallback_builder->CreateFontFallback(m_fallback.put());
        }
    }

    void AddFallbacks(IDWriteTextLayout2 *layout) {
        if (layout && m_fallback) {
            check_hresult(layout->SetFontFallback(m_fallback.get()));
        }
    }

    com_ptr<ID2D1Factory1> m_d2d1 = nullptr;
    com_ptr<IDWriteFactory3> m_dwrite = nullptr;
    com_ptr<IWICImagingFactory> m_wic = nullptr;
    com_ptr<IDWriteFontCollection> m_fontcollection = nullptr;
    com_ptr<IDWriteFontFallback> m_fallback = nullptr;
    com_ptr<FontLoader> m_loader = nullptr;
};

} // namespace

com_ptr<RenderFactoryImpl> g_factory = nullptr;

com_ptr<RenderFactory> RenderFactory::Create() {
    auto impl = make_self<RenderFactoryImpl>();
    impl->Initialize();
    return impl;

    if (!g_factory) {
        g_factory = make_self<RenderFactoryImpl>();
        g_factory->Initialize();
    }

    return g_factory;
}

void RenderFactory::Shutdown() {
    if (g_factory) {
        g_factory->Uninitialize();
        g_factory = nullptr;
    }
}

} // namespace khiin::win32
