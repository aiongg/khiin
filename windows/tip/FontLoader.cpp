#include "pch.h"

#include "Config.h"
#include "FontLoader.h"
#include "RenderFactory.h"

namespace khiin::win32 {
namespace {
using namespace winrt;

struct WriteFontFileEnumerator : implements<WriteFontFileEnumerator, IDWriteFontFileEnumerator> {
    using files_t = std::vector<std::wstring>;
    using iter_t = files_t::const_iterator;

    WriteFontFileEnumerator(IDWriteFactory *factory) {
        m_factory.copy_from(factory);
        m_files = Config::GetFontFiles();

        if (!m_files.empty()) {
            m_iterator = m_files.cbegin();
        } else {
            m_iterator = m_files.cend();
        }
    }

    STDMETHODIMP MoveNext(BOOL *hasCurrentFile) override {
        m_current_file = nullptr;
        *hasCurrentFile = FALSE;

        if (m_files.empty()) {
            return S_OK;
        }

        if (!m_initialized) {
            m_initialized = true;
        } else if (m_iterator != m_files.cend()) {
            ++m_iterator;
        }

        while (m_iterator != m_files.cend()) {
            auto const &font = m_iterator[0];
            auto hr = m_factory->CreateFontFileReference(m_iterator[0].c_str(), NULL, m_current_file.put());
            if (hr == S_OK) {
                *hasCurrentFile = TRUE;
                return S_OK;
            }
            ++m_iterator;
        }

        return S_OK;
    }

    STDMETHODIMP GetCurrentFontFile(IDWriteFontFile **fontFile) override {
        if (m_current_file) {
            auto names = com_ptr<IDWriteLocalizedStrings>();
            m_current_file.copy_to(fontFile);
            return S_OK;
        }

        return E_FAIL;
    }

  private:
    com_ptr<IDWriteFactory> m_factory = nullptr;
    com_ptr<IDWriteFontFile> m_current_file = nullptr;
    bool m_initialized = false;
    files_t m_files;
    iter_t m_iterator;
};

} // namespace

// std::vector<std::wstring> FontLoader::AvailableFontFamilies() {
//    auto factory = RenderFactory::Create();
//    auto loader = Instance();
//    auto collection = com_ptr<IDWriteFontCollection>();
//    auto hr = factory->DWrite3()->CreateCustomFontCollection(loader.get(), FontLoader::collection_key,
//                                                             static_cast<UINT32>(strlen(FontLoader::collection_key)),
//                                                             collection.put());
//    auto family_count = collection->GetFontFamilyCount();
//    auto ret = std::vector<std::wstring>();
//
//    for (UINT32 i = 0; i < family_count; ++i) {
//        auto family = com_ptr<IDWriteFontFamily>();
//        hr = collection->GetFontFamily(i, family.put());
//
//        if (FAILED(hr)) {
//            continue;
//        }
//
//        auto names = com_ptr<IDWriteLocalizedStrings>();
//        UINT32 name_index = 0;
//        BOOL exists = FALSE;
//        hr = family->GetFamilyNames(names.put());
//
//        if (SUCCEEDED(hr)) {
//            wchar_t locale_name[LOCALE_NAME_MAX_LENGTH] = {};
//            int success = ::GetUserDefaultLocaleName(locale_name, LOCALE_NAME_MAX_LENGTH);
//
//            if (success != 0) {
//                hr = names->FindLocaleName(locale_name, &name_index, &exists);
//            }
//
//            if (SUCCEEDED(hr) && exists == FALSE) {
//                hr = names->FindLocaleName(L"en-us", &name_index, &exists);
//            }
//        }
//
//        if (exists == FALSE) {
//            name_index = 0;
//        }
//
//        UINT32 name_size = 0;
//
//        hr = names->GetStringLength(name_index, &name_size);
//
//        if (FAILED(hr)) {
//            continue;
//        }
//
//        auto name = std::wstring();
//        name.resize(name_size + 1);
//        hr = names->GetString(name_index, &name[0], name_size + 1);
//
//        if (FAILED(hr)) {
//            continue;
//        }
//
//        KHIIN_DEBUG(L"Found font family: {}\n", name);
//
//        ret.push_back(std::move(name));
//    }
//
//    return ret;
//}

//+---------------------------------------------------------------------------
//
// IDWriteFontCollectionLoader
//
//----------------------------------------------------------------------------

STDMETHODIMP
FontLoader::CreateEnumeratorFromKey(IDWriteFactory *factory, void const *collectionKey, UINT32 collectionKeySize,
                                    IDWriteFontFileEnumerator **fontFileEnumerator) {
    if (collectionKeySize != strlen(collection_key)) {
        return E_INVALIDARG;
    }

    if (strcmp((const char *)collectionKey, collection_key) != 0) {
        return E_INVALIDARG;
    }

    auto enumerator = make<WriteFontFileEnumerator>(factory);
    enumerator.copy_to(fontFileEnumerator);

    return S_OK;
}

com_ptr<FontLoader> FontLoader::Create() {
    return make_self<FontLoader>();
}

} // namespace khiin::win32
