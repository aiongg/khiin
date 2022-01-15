#pragma once

namespace Khiin {

struct DisplayAttributeInfo : winrt::implements<DisplayAttributeInfo, ITfDisplayAttributeInfo> {
    DisplayAttributeInfo() = default;
    DisplayAttributeInfo(const DisplayAttributeInfo &) = delete;
    DisplayAttributeInfo &operator=(const DisplayAttributeInfo &) = delete;
    ~DisplayAttributeInfo() = default;

    HRESULT init(std::wstring description, winrt::guid guid, TF_DISPLAYATTRIBUTE attribute);

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
