#pragma once

#include "DisplayAttributeInfo.h"

namespace Khiin {

enum class AttrInfoKey {
    Input = 0,
    Converted,
};

struct DisplayAttributeInfoEnum : winrt::implements<DisplayAttributeInfoEnum, IEnumTfDisplayAttributeInfo> {
    DisplayAttributeInfoEnum() = default;
    DisplayAttributeInfoEnum(const DisplayAttributeInfoEnum &) = delete;
    DisplayAttributeInfoEnum &operator=(const DisplayAttributeInfoEnum &) = delete;
    ~DisplayAttributeInfoEnum() = default;

    using DisplayAttributeInfos = std::map<AttrInfoKey, winrt::com_ptr<DisplayAttributeInfo>>;

    static void load(_Out_ DisplayAttributeInfoEnum **ppInfoEnum);

    void addAttribute(AttrInfoKey key, DisplayAttributeBundle attr);
    void addAttribute(AttrInfoKey key, winrt::com_ptr<DisplayAttributeInfo> attr);
    HRESULT at(AttrInfoKey index, _Out_ ITfDisplayAttributeInfo **pDaInfo);
    HRESULT findByGuid(REFGUID guid, _Outptr_opt_ ITfDisplayAttributeInfo **ppInfo);

    // IEnumTfDisplayAttributeInfo
    virtual STDMETHODIMP Clone(_Out_ IEnumTfDisplayAttributeInfo **ppEnumInfo) override;
    virtual STDMETHODIMP Next(ULONG ulCount,
                              __RPC__out_ecount_part(ulCount, *pcFetched) ITfDisplayAttributeInfo **rgInfo,
                              _Out_opt_ ULONG *pcFetched) override;
    virtual STDMETHODIMP Reset(void) override;
    virtual STDMETHODIMP Skip(ULONG ulCount) override;

  private:
    DisplayAttributeInfos attributes;
    DisplayAttributeInfos::const_iterator attr_iterator = attributes.cbegin();
};

} // namespace Khiin
