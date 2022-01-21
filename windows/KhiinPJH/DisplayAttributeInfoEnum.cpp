#include "pch.h"

#include "DisplayAttributeInfoEnum.h"

#include "DisplayAttributeInfo.h"

namespace khiin::win32 {

void DisplayAttributeInfoEnum::load(_Out_ DisplayAttributeInfoEnum **ppDaiiEnum) {
    auto infoEnum = winrt::make_self<DisplayAttributeInfoEnum>();
    infoEnum->addAttribute(AttrInfoKey::Input, DisplayAttribute_Input);
    infoEnum->addAttribute(AttrInfoKey::Converted, DisplayAttribute_Converted);
    infoEnum.copy_to(ppDaiiEnum);
}

void DisplayAttributeInfoEnum::addAttribute(AttrInfoKey key, DisplayAttributeBundle bundle) {
    auto attrInfo = winrt::make_self<DisplayAttributeInfo>();
    attrInfo->Initialize(bundle);
    attributes[key] = std::move(attrInfo);
}

void DisplayAttributeInfoEnum::addAttribute(AttrInfoKey key, winrt::com_ptr<DisplayAttributeInfo> attr) {
    attributes[key] = attr;
}

void DisplayAttributeInfoEnum::at(AttrInfoKey index, ITfDisplayAttributeInfo **pInfo) {
    try {
        attributes.at(index).as<ITfDisplayAttributeInfo>().copy_to(pInfo);
    } catch (...) {
        throw winrt::hresult_error(ERROR_NOT_FOUND);
    }
}

void DisplayAttributeInfoEnum::findByGuid(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) {
    for (const auto &[index, attr] : attributes) {
        if (attr->getGuid() == guid) {
            attr.as<ITfDisplayAttributeInfo>().copy_to(ppInfo);
            return;
        }
    }
}

//+---------------------------------------------------------------------------
//
// IEnumTfDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDMETHODIMP DisplayAttributeInfoEnum::Clone(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo **ppEnumInfo) {
    auto daiEnum = winrt::make_self<DisplayAttributeInfoEnum>();
    for (const auto &[index, attr] : attributes) {
        auto clone = winrt::make_self<DisplayAttributeInfo>();
        attr->clone(clone.put());
        daiEnum->addAttribute(index, clone);
    }
    daiEnum.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnumInfo);
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfoEnum::Next(ULONG ulCount,
                                            __RPC__out_ecount_part(ulCount, *pcFetched)
                                                ITfDisplayAttributeInfo **rgInfo,
                                            __RPC__out ULONG *pcFetched) {
    auto i = ULONG(0);
    auto end = attributes.cend();
    auto nAttrs = attributes.size();
    for (; i < ulCount; ++i) {
        if (attr_iterator == end) {
            break;
        }
        rgInfo[i] = attr_iterator->second.as<ITfDisplayAttributeInfo>().get();
        ++attr_iterator;
    }
    if (pcFetched) {
        *pcFetched = i;
    }
    return i == ulCount ? S_OK : S_FALSE;
}

STDMETHODIMP DisplayAttributeInfoEnum::Reset(void) {
    attr_iterator = attributes.cbegin();
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfoEnum::Skip(ULONG ulCount) {
    try {
        std::advance(attr_iterator, ulCount);
        return S_OK;
    } catch (...) {
        return S_FALSE;
    }
}

} // namespace khiin::win32