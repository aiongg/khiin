#pragma once

#include "DisplayAttributeInfo.h"

// namespace Khiin {
// struct __declspec(uuid("829893fb-728d-11ec-8c6e-e0d46491b35a")) DisplayAttributeInfoEnum;
//}

// namespace winrt {
//
// template <>
// inline bool is_guid_of<Khiin::DisplayAttributeInfoEnum>(guid const &id) noexcept {
//    return is_guid_of<Khiin::DisplayAttributeInfoEnum, IEnumTfDisplayAttributeInfo>(id);
//}
//
//} // namespace winrt

namespace Khiin {

enum class AttrInfoKey {
    Input = 0,
    Converted,
};

// struct __declspec(uuid("829893fb-728d-11ec-8c6e-e0d46491b35a"))
struct DisplayAttributeInfoEnum : winrt::implements<DisplayAttributeInfoEnum, IEnumTfDisplayAttributeInfo> {
    DisplayAttributeInfoEnum() = default;
    DisplayAttributeInfoEnum(const DisplayAttributeInfoEnum &) = delete;
    DisplayAttributeInfoEnum &operator=(const DisplayAttributeInfoEnum &) = delete;
    ~DisplayAttributeInfoEnum() = default;

    using DisplayAttributeInfos = std::map<AttrInfoKey, winrt::com_ptr<DisplayAttributeInfo>>;

    static void load(_Out_ DisplayAttributeInfoEnum **ppInfoEnum);

    void addAttribute(AttrInfoKey key, DisplayAttributeBundle attr);
    void addAttribute(AttrInfoKey key, winrt::com_ptr<DisplayAttributeInfo> attr);
    void at(AttrInfoKey index, ITfDisplayAttributeInfo **pDaInfo);
    void findByGuid(REFGUID guid, ITfDisplayAttributeInfo **ppInfo);

    // IEnumTfDisplayAttributeInfo
    virtual STDMETHODIMP Clone(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo **ppEnum) override;
    virtual STDMETHODIMP Next(ULONG ulCount,
                              __RPC__out_ecount_part(ulCount, *pcFetched) ITfDisplayAttributeInfo **rgInfo,
                              __RPC__out ULONG *pcFetched) override;
    virtual STDMETHODIMP Reset(void) override;
    virtual STDMETHODIMP Skip(ULONG ulCount) override;

  private:
    DisplayAttributeInfos attributes;
    DisplayAttributeInfos::const_iterator attr_iterator = attributes.cbegin();
};

} // namespace Khiin
