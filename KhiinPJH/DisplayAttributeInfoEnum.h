#pragma once

#include "DisplayAttributeInfo.h"

namespace Khiin {

struct DisplayAttributeInfoEnum : winrt::implements<DisplayAttributeInfoEnum, IEnumTfDisplayAttributeInfo> {
    using DisplayAttributeInfos = std::vector<winrt::com_ptr<DisplayAttributeInfo>>;

    DisplayAttributeInfoEnum() = default;
    DisplayAttributeInfoEnum(const DisplayAttributeInfoEnum &) = delete;
    DisplayAttributeInfoEnum &operator=(const DisplayAttributeInfoEnum &) = delete;
    ~DisplayAttributeInfoEnum() = default;

    //static HRESULT enumerate(DisplayAttributeInfos &attributeInfos);

    void addAttribute(DisplayAttributeBundle attr);
    void addAttribute(winrt::com_ptr<DisplayAttributeInfo> attr);

    // IEnumTfDisplayAttributeInfo
    virtual STDMETHODIMP Clone(IEnumTfDisplayAttributeInfo **ppEnum) override;
    virtual STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched) override;
    virtual STDMETHODIMP Reset(void) override;
    virtual STDMETHODIMP Skip(ULONG ulCount) override;

  private:
    DisplayAttributeInfos displayAttributes;
};

} // namespace Khiin
