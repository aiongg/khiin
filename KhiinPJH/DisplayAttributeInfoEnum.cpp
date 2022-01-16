#include "pch.h"

#include "DisplayAttributeInfoEnum.h"

#include "DisplayAttributeInfo.h"

namespace Khiin {

void DisplayAttributeInfoEnum::load(_Out_ DisplayAttributeInfoEnum **ppDaiiEnum) {
    auto daiiEnum = winrt::make_self<DisplayAttributeInfoEnum>();
    daiiEnum->addAttribute(DisplayAttribute_Input);     // AttributeIndex = 0
    daiiEnum->addAttribute(DisplayAttribute_Converted); // AttributeIndex = 1
    daiiEnum.copy_to(ppDaiiEnum);
}

void DisplayAttributeInfoEnum::addAttribute(DisplayAttributeBundle attrBundle) {
    auto attrInfo = winrt::make_self<DisplayAttributeInfo>();
    attrInfo->init(attrBundle);
    attributes.push_back(std::move(attrInfo));
}

void DisplayAttributeInfoEnum::addAttribute(winrt::com_ptr<DisplayAttributeInfo> attr) {
    attributes.push_back(attr);
}

void DisplayAttributeInfoEnum::at(AttributeIndex index, _Out_ ITfDisplayAttributeInfo **pInfo) {
    attributes.at(static_cast<int>(index)).as<ITfDisplayAttributeInfo>().copy_to(pInfo);
}

HRESULT DisplayAttributeInfoEnum::findByGuid(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) {
    for (const auto &attr : attributes) {
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

STDMETHODIMP DisplayAttributeInfoEnum::Clone(IEnumTfDisplayAttributeInfo **ppEnum) {
    auto daiEnum = winrt::make_self<DisplayAttributeInfoEnum>();
    for (auto &attr : attributes) {
        auto clone = winrt::make_self<DisplayAttributeInfo>();
        attr->clone(clone.put());
        daiEnum->addAttribute(clone);
    }
    daiEnum.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfoEnum::Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched) {
    auto i = ULONG(0);
    auto nAttrs = attributes.size();
    for (; i < ulCount; ++i) {
        if (currentIndex >= nAttrs) {
            break;
        }
        rgInfo[i] = attributes.at(currentIndex).as<ITfDisplayAttributeInfo>().get();
        ++currentIndex;
    }
    if (pcFetched) {
        *pcFetched = i;
    }
    return i == ulCount ? S_OK : S_FALSE;
}

STDMETHODIMP DisplayAttributeInfoEnum::Reset(void) {
    currentIndex = 0;
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfoEnum::Skip(ULONG ulCount) {
    currentIndex += ulCount;
    return currentIndex > attributes.size() ? S_FALSE : S_OK;
}

} // namespace Khiin