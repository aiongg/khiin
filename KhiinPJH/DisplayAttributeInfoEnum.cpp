#include "pch.h"

#include "DisplayAttributeInfoEnum.h"

#include "DisplayAttributeInfo.h"

namespace Khiin {

HRESULT DisplayAttributeInfoEnum::enumerate(DisplayAttributes &attrs) {
    attrs.clear();

    auto attr1 = winrt::make_self<DisplayAttributeInfo>();
    attr1->init(inputAttrDesc, inputAttrGuid, inputAttr);
    attrs.push_back(attr1);

    auto attr2 = winrt::make_self<DisplayAttributeInfo>();
    attr2->init(convertedAttrDesc, convertedAttrGuid, convertedAttr);
    attrs.push_back(attr2);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// IEnumTfDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDMETHODIMP DisplayAttributeInfoEnum::Clone(IEnumTfDisplayAttributeInfo **ppEnum) {
    return E_NOTIMPL;
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