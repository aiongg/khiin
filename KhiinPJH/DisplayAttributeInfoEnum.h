#pragma once

#include "DisplayAttributeInfo.h"

namespace Khiin {

enum class AttributeIndex {
    Input = 0,
    Converted,
};

struct DisplayAttributeInfoEnum : winrt::implements<DisplayAttributeInfoEnum, IEnumTfDisplayAttributeInfo> {
    using DisplayAttributeInfos = std::vector<winrt::com_ptr<DisplayAttributeInfo>>;

    static void load(_Out_ DisplayAttributeInfoEnum **ppDaiiEnum);

    DisplayAttributeInfoEnum() = default;
    DisplayAttributeInfoEnum(const DisplayAttributeInfoEnum &) = delete;
    DisplayAttributeInfoEnum &operator=(const DisplayAttributeInfoEnum &) = delete;
    ~DisplayAttributeInfoEnum() = default;

    // static HRESULT enumerate(DisplayAttributeInfos &attributeInfos);

    void addAttribute(DisplayAttributeBundle attr);
    void addAttribute(winrt::com_ptr<DisplayAttributeInfo> attr);
    void at(AttributeIndex index, _Out_ ITfDisplayAttributeInfo **pDaInfo);
    HRESULT findByGuid(REFGUID guid, ITfDisplayAttributeInfo **ppInfo);

    // IEnumTfDisplayAttributeInfo
    virtual STDMETHODIMP Clone(IEnumTfDisplayAttributeInfo **ppEnum) override;
    virtual STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched) override;
    virtual STDMETHODIMP Reset(void) override;
    virtual STDMETHODIMP Skip(ULONG ulCount) override;

  private:
    DisplayAttributeInfos attributes;
    DisplayAttributeInfos::size_type currentIndex = 0;
};

} // namespace Khiin
