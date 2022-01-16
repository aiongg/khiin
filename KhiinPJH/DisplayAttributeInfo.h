#pragma once

namespace Khiin {

struct DisplayAttributeBundle {
    std::wstring description;
    winrt::guid guid;
    TF_DISPLAYATTRIBUTE attribute;
};

// 829893f8-728d-11ec-8c6e-e0d46491b35a
extern const DisplayAttributeBundle DisplayAttribute_Input;

// 829893f9-728d-11ec-8c6e-e0d46491b35a
extern const DisplayAttributeBundle DisplayAttribute_Converted;

struct DisplayAttributeInfo : winrt::implements<DisplayAttributeInfo, ITfDisplayAttributeInfo> {
    DisplayAttributeInfo() = default;
    DisplayAttributeInfo(const DisplayAttributeInfo &) = delete;
    DisplayAttributeInfo &operator=(const DisplayAttributeInfo &) = delete;
    ~DisplayAttributeInfo() = default;

    HRESULT init(DisplayAttributeBundle &bundle);
    HRESULT clone(DisplayAttributeInfo **ppInfo);
    GUID getGuid();

    // ITfDisplayAttributeInfo
    virtual STDMETHODIMP GetGUID(GUID *pguid) override;
    virtual STDMETHODIMP GetDescription(BSTR *pbstrDesc) override;
    virtual STDMETHODIMP GetAttributeInfo(TF_DISPLAYATTRIBUTE *pda) override;
    virtual STDMETHODIMP SetAttributeInfo(const TF_DISPLAYATTRIBUTE *pda) override;
    virtual STDMETHODIMP Reset(void) override;

  private:
    std::wstring description{};
    winrt::guid guid{};
    TF_DISPLAYATTRIBUTE attribute = TF_DISPLAYATTRIBUTE{};
    TF_DISPLAYATTRIBUTE attributeBackup = TF_DISPLAYATTRIBUTE{};
};

} // namespace Khiin
