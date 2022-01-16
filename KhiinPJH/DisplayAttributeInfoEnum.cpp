#include "pch.h"

#include "DisplayAttributeInfoEnum.h"

#include "DisplayAttributeInfo.h"

namespace Khiin {

void DisplayAttributeInfoEnum::load(_Out_ DisplayAttributeInfoEnum **ppDaiiEnum) {
    auto infoEnum = winrt::make_self<DisplayAttributeInfoEnum>();
    infoEnum->addAttribute(AttrInfoKey::Input, DisplayAttribute_Input);
    infoEnum->addAttribute(AttrInfoKey::Converted, DisplayAttribute_Converted);
    infoEnum.copy_to(ppDaiiEnum);
}

void DisplayAttributeInfoEnum::addAttribute(AttrInfoKey key, DisplayAttributeBundle bundle) {
    auto attrInfo = winrt::make_self<DisplayAttributeInfo>();
    attrInfo->init(bundle);
    attributes[key] = std::move(attrInfo);
}

void DisplayAttributeInfoEnum::addAttribute(AttrInfoKey key, winrt::com_ptr<DisplayAttributeInfo> attr) {
    attributes[key] = attr;
}

HRESULT DisplayAttributeInfoEnum::at(AttrInfoKey index, _Out_ ITfDisplayAttributeInfo **pInfo) {
    try {
        attributes.at(index).as<ITfDisplayAttributeInfo>().copy_to(pInfo);
        return S_OK;
    } catch (...) {
        return ERROR_NOT_FOUND;
    }
}

HRESULT DisplayAttributeInfoEnum::findByGuid(REFGUID guid, _Outptr_opt_ ITfDisplayAttributeInfo **ppInfo) {
    for (const auto &[index, attr] : attributes) {
        if (attr->getGuid() == guid) {
            attr.as<ITfDisplayAttributeInfo>().copy_to(ppInfo);
            return S_OK;
        }
    }

    return ERROR_NOT_FOUND;
}

//+---------------------------------------------------------------------------
//
// IEnumTfDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDMETHODIMP DisplayAttributeInfoEnum::Clone(_Out_ IEnumTfDisplayAttributeInfo **ppEnumInfo) {
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
                                            _Out_opt_ ULONG *pcFetched) {
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

} // namespace Khiin