#pragma once

#include <string>
#include <vector>

namespace khiin::win32 {

struct FontLoader : winrt::implements<FontLoader, IDWriteFontCollectionLoader> {
    static winrt::com_ptr<FontLoader> Create();

    static inline constexpr const char *collection_key = "KHIIN_FONT_COLLECTION";

    //static std::vector<std::wstring> AvailableFontFamilies();

    //+---------------------------------------------------------------------------
    //
    // IDWriteFontCollectionLoader
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP CreateEnumeratorFromKey(IDWriteFactory *factory, void const *collectionKey, UINT32 collectionKeySize,
                                         IDWriteFontFileEnumerator **fontFileEnumerator) override;
};

} // namespace khiin::win32
