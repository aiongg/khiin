#include "pch.h"

#include "LangBarIndicator.h"

#include <unordered_map>

namespace khiin::win32 {
namespace {
using namespace winrt;

struct LangBarIndicatorImpl : implements<LangBarIndicatorImpl, ITfSource, ITfLangBarItemButton, LangBarIndicator> {

    //+---------------------------------------------------------------------------
    //
    // ITfSource
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP AdviseSink(REFIID riid, IUnknown *unknown, DWORD *cookie) override {
        TRY_FOR_HRESULT;

        if (::IsEqualIID(riid, IID_ITfLangBarItemSink)) {
            auto lang_bar_item_sink = com_ptr<ITfLangBarItemSink>();
            check_hresult(unknown->QueryInterface(IID_ITfLangBarItemSink, lang_bar_item_sink.put_void()));
            *cookie = static_cast<DWORD>(rand());
            m_sinks[*cookie] = std::move(lang_bar_item_sink);
        } else {
            return E_INVALIDARG;
        }

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP UnadviseSink(DWORD dwCookie) override {
        TRY_FOR_HRESULT;

        if (auto it = m_sinks.find(dwCookie); it != m_sinks.end()) {
            m_sinks.erase(it);
        }

        CATCH_FOR_HRESULT;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfLangBarItemButton
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP GetInfo(TF_LANGBARITEMINFO *pInfo) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetStatus(DWORD *pdwStatus) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP Show(BOOL fShow) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetTooltipString(BSTR *pbstrToolTip) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT *prcArea) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP InitMenu(ITfMenu *pMenu) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP OnMenuSelect(UINT wID) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetIcon(HICON *phIcon) override {
        return E_NOTIMPL;
    }
    virtual STDMETHODIMP GetText(BSTR *pbstrText) override {
        return E_NOTIMPL;
    }

  private:
    std::unordered_map<DWORD, com_ptr<ITfLangBarItemSink>> m_sinks;
};

} // namespace

} // namespace khiin::win32
