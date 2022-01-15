#include "pch.h"

#include "DisplayAttributeInfo.h"

namespace Khiin {

HRESULT DisplayAttributeInfo::init(std::wstring description, winrt::guid guid, TF_DISPLAYATTRIBUTE attribute) {
    this->description = description;
    this->guid = guid;
    this->attribute = attribute;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDMETHODIMP DisplayAttributeInfo::GetGUID(GUID *pguid) {
    *pguid = guid;
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfo::GetDescription(BSTR *pbstrDesc) {
    *pbstrDesc = ::SysAllocString(description.c_str());
    return *pbstrDesc ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP DisplayAttributeInfo::GetAttributeInfo(TF_DISPLAYATTRIBUTE *pda) {
    *pda = attribute;
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfo::SetAttributeInfo(const TF_DISPLAYATTRIBUTE *pda) {
    attribute = *pda;
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfo::Reset(void) {
    attribute = attributeBackup;
    return S_OK;
}

} // namespace Khiin
