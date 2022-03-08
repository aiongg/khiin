#include "pch.h"

#include "LangBarIndicator.h"

#include <unordered_map>

#include "BaseWindow.h"
#include "Config.h"
#include "DllModule.h"
#include "PopupMenu.h"
#include "Profile.h"
#include "TextService.h"
#include "common.h"

namespace khiin::win32 {
namespace {
using namespace winrt;
using namespace messages;

volatile HMODULE g_module;

struct LangBarIndicatorImpl :
    implements<LangBarIndicatorImpl, ITfSource, ITfLangBarItemButton, LangBarIndicator>,
    ConfigChangeListener {
    LangBarIndicatorImpl() {
        m_info.clsidService = Profile::textServiceGuid;
        m_info.guidItem = GUID_LBI_INPUTMODE;
        m_info.dwStyle = TF_LBI_STYLE_BTN_BUTTON;
        m_info.ulSort = 0;
    }

    virtual void Initialize(TextService *pService) override {
        m_service.copy_from(pService);
        auto langbarmgr = langbar_item_mgr();
        check_hresult(langbarmgr->AddItem(this));
        m_popup = std::unique_ptr<PopupMenu>(PopupMenu::Create(m_service.get()));
        OnConfigChanged(m_service->config());
        m_service->RegisterConfigChangeListener(this);
    }

    virtual void Shutdown() override {
        auto langbarmgr = langbar_item_mgr();
        check_hresult(langbarmgr->RemoveItem(this));
        m_service = nullptr;
        if (m_popup && m_popup->hwnd()) {
            ::DestroyWindow(m_popup->hwnd());
        }
        m_popup.reset();
    }

    com_ptr<ITfLangBarItemMgr> langbar_item_mgr() {
        auto threadmgr = m_service->thread_mgr();
        auto langbarmgr = com_ptr<ITfLangBarItemMgr>();
        check_hresult(threadmgr->QueryInterface(IID_ITfLangBarItemMgr, langbarmgr.put_void()));
        return langbarmgr;
    }

    //+---------------------------------------------------------------------------
    //
    // ConfigChangeListener
    //
    //----------------------------------------------------------------------------

    virtual void OnConfigChanged(AppConfig *config) override {
        if (m_popup) {
            m_popup->OnConfigChanged(config);
        }
    }

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
        *pInfo = m_info;
        return S_OK;
    }

    virtual STDMETHODIMP GetStatus(DWORD *pdwStatus) override {
        *pdwStatus = m_status;
        return S_OK;
    }

    virtual STDMETHODIMP Show(BOOL fShow) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetTooltipString(BSTR *pbstrToolTip) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT *prcArea) override {
        KHIIN_DEBUG("Clicked: ({},{})", pt.x, pt.y);
        m_popup->Show(pt);

        return S_OK;
    }

    virtual STDMETHODIMP InitMenu(ITfMenu *pMenu) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP OnMenuSelect(UINT wID) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetIcon(HICON *phIcon) override {
        TRY_FOR_HRESULT
        using namespace messages;

        auto icon_resource = 0;

        switch (m_service->config()->input_mode()) {
        case IM_CONTINUOUS:
            icon_resource = IDI_MODE_CONTINUOUS;
            break;
        case IM_BASIC:
            icon_resource = IDI_MODE_BASIC;
            break;
        case IM_PRO:
            icon_resource = IDI_MODE_PRO;
            break;
        case IM_ALPHA:
            icon_resource = IDI_MODE_ALPHA;
            break;
        default:
            return E_INVALIDARG;
        }

        auto icon =
            ::LoadImage(m_service->hmodule(), MAKEINTRESOURCE(icon_resource), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);

        *phIcon = static_cast<HICON>(icon);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetText(BSTR *pbstrText) override {
        return E_NOTIMPL;
    }

  private:
    com_ptr<TextService> m_service = nullptr;
    std::unordered_map<DWORD, com_ptr<ITfLangBarItemSink>> m_sinks;
    std::unique_ptr<PopupMenu> m_popup = nullptr;

    TF_LANGBARITEMINFO m_info;
    DWORD m_status = 0;
};

} // namespace

void LangBarIndicatorFactory::Create(LangBarIndicator **indicator) {
    as_self<LangBarIndicator>(make_self<LangBarIndicatorImpl>()).copy_to(indicator);
}

} // namespace khiin::win32
