#include "pch.h"

#include "DisplayAttributeInfoEnum.h"

#include "DisplayAttributeInfo.h"

namespace Khiin {

void DisplayAttributeInfoEnum::addAttribute(DisplayAttributeBundle attrBundle) {
    auto attrInfo = winrt::make_self<DisplayAttributeInfo>();
    attrInfo->init(attrBundle);
    displayAttributes.push_back(std::move(attrInfo));
}

void DisplayAttributeInfoEnum::addAttribute(winrt::com_ptr<DisplayAttributeInfo> attr) {
    displayAttributes.push_back(attr);
}

//+---------------------------------------------------------------------------
//
// IEnumTfDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDMETHODIMP DisplayAttributeInfoEnum::Clone(IEnumTfDisplayAttributeInfo **ppEnum) {
    auto daiEnum = winrt::make_self<DisplayAttributeInfoEnum>();
    for (auto &attr : displayAttributes) {
        auto clone = winrt::make_self<DisplayAttributeInfo>();
        attr->clone(clone.put());
        daiEnum->addAttribute(clone);
    }
    daiEnum.as<IEnumTfDisplayAttributeInfo>().copy_to(ppEnum);
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfoEnum::Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched) {
    return E_NOTIMPL;
}

STDMETHODIMP DisplayAttributeInfoEnum::Reset(void) {
    return E_NOTIMPL;
}

STDMETHODIMP DisplayAttributeInfoEnum::Skip(ULONG ulCount) {
    return E_NOTIMPL;
}

} // namespace Khiin