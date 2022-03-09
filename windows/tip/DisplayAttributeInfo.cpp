#include "pch.h"

#include "DisplayAttributeInfo.h"

#include "Guids.h"

namespace khiin::win32::tip {

const DisplayAttributeBundle DisplayAttribute_Input =
    DisplayAttributeBundle{L"Input",
                           guids::kDisplayAttrInput,
                           TF_DISPLAYATTRIBUTE{
                               {TF_CT_NONE, 0}, // text color
                               {TF_CT_NONE, 0}, // background color
                               TF_LS_SQUIGGLE,  // underline style
                               FALSE,           // underline boldness
                               {TF_CT_NONE, 0}, // underline color
                               TF_ATTR_INPUT    // attribute info
                           }};

const DisplayAttributeBundle DisplayAttribute_Converted =
    DisplayAttributeBundle{L"Converted",
                           guids::kDisplayAttrConverted,
                           TF_DISPLAYATTRIBUTE{
                               {TF_CT_NONE, 0},  // text color
                               {TF_CT_NONE, 0},  // background color
                               TF_LS_SOLID,      // underline style
                               FALSE,            // underline boldness
                               {TF_CT_NONE, 0},  // underline color
                               TF_ATTR_CONVERTED // attribute info
                           }};

const DisplayAttributeBundle DisplayAttribute_Focused =
    DisplayAttributeBundle{L"Focused",
                           guids::kDisplayAttrFocused,
                           TF_DISPLAYATTRIBUTE{
                               {TF_CT_NONE, 0},         // text color
                               {TF_CT_NONE, 0},         // background color
                               TF_LS_SOLID,             // underline style
                               TRUE,                    // underline boldness
                               {TF_CT_NONE, 0},         // underline color
                               TF_ATTR_TARGET_CONVERTED // attribute info
                           }};

HRESULT DisplayAttributeInfo::Initialize(DisplayAttributeBundle &bundle) {
    description = bundle.description;
    guid = bundle.guid;
    attribute = bundle.attribute;
    attributeBackup = bundle.attribute;
    return S_OK;
}

HRESULT DisplayAttributeInfo::clone(DisplayAttributeInfo **ppDaInfo) {
    auto clone = winrt::make_self<DisplayAttributeInfo>();
    auto bundle = DisplayAttributeBundle{description, guid, attributeBackup};
    clone->Initialize(bundle);
    clone.copy_to(ppDaInfo);
    return S_OK;
}

GUID DisplayAttributeInfo::getGuid() {
    return guid;
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

} // namespace khiin::win32
