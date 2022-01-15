#pragma once

namespace Khiin {

struct DisplayAttributeInfoEnum : winrt::implements<DisplayAttributeInfoEnum, IEnumTfDisplayAttributeInfo> {
    using DisplayAttributes = std::vector<winrt::com_ptr<ITfDisplayAttributeInfo>>;

    DisplayAttributeInfoEnum() = default;
    DisplayAttributeInfoEnum(const DisplayAttributeInfoEnum &) = delete;
    DisplayAttributeInfoEnum &operator=(const DisplayAttributeInfoEnum &) = delete;
    ~DisplayAttributeInfoEnum() = default;

    static HRESULT enumerate(DisplayAttributes &attrs);

    static inline const std::wstring inputAttrDesc = L"Input";
    static inline const winrt::guid inputAttrGuid // 829893f8-728d-11ec-8c6e-e0d46491b35a
        {0x829893f8, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};
    static inline const TF_DISPLAYATTRIBUTE inputAttr{
        {TF_CT_NONE, 0}, // text color
        {TF_CT_NONE, 0}, // background color
        TF_LS_SQUIGGLE,  // underline style
        FALSE,           // underline boldness
        {TF_CT_NONE, 0}, // underline color
        TF_ATTR_INPUT    // attribute info
    };

    static inline const std::wstring convertedAttrDesc = L"Converted";
    static inline const winrt::guid convertedAttrGuid // 829893f9-728d-11ec-8c6e-e0d46491b35a
        {0x829893f9, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};
    static inline const TF_DISPLAYATTRIBUTE convertedAttr{
        {TF_CT_NONE, 0},         // text color
        {TF_CT_NONE, 0},         // background color
        TF_LS_SOLID,             // underline style
        FALSE,                   // underline boldness
        {TF_CT_NONE, 0},         // underline color
        TF_ATTR_TARGET_CONVERTED // attribute info
    };

    // IEnumTfDisplayAttributeInfo
    virtual STDMETHODIMP Clone(IEnumTfDisplayAttributeInfo **ppEnum) override;
    virtual STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched) override;
    virtual STDMETHODIMP Reset(void) override;
    virtual STDMETHODIMP Skip(ULONG ulCount) override;
};

} // namespace Khiin
