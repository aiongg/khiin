#include "pch.h"

#include "LangBarIndicator.h"

#include <unordered_map>

#include "BaseWindow.h"
#include "Config.h"
#include "DllModule.h"
#include "Guids.h"
#include "PopupMenu.h"
#include "Profile.h"
#include "TextService.h"
#include "proto/proto.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;
using namespace proto;

volatile HMODULE g_module;

struct LangBarIndicatorImpl :
    implements<LangBarIndicatorImpl, ITfSource, ITfLangBarItemButton, LangBarIndicator>,
    ConfigChangeListener {
    LangBarIndicatorImpl() {
        m_info.clsidService = guids::kTextService;
        m_info.guidItem = GUID_LBI_INPUTMODE;
        m_info.dwStyle = TF_LBI_STYLE_BTN_BUTTON;
        m_info.ulSort = 0;
    }

    void Initialize(TextService *service) override {
        m_service.copy_from(service);
        auto langbarmgr = langbar_item_mgr();
        check_hresult(langbarmgr->AddItem(this));
        m_popup = std::unique_ptr<PopupMenu>(PopupMenu::Create(m_service.get()));
        OnConfigChanged(m_service->config());
        m_service->RegisterConfigChangeListener(this);
    }

    void Shutdown() override {
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

    void OnConfigChanged(AppConfig *config) override {
        for (auto &sink : m_sinks) {
            sink.second->OnUpdate(TF_LBI_ICON);
        }
        if (m_popup) {
            m_popup->OnConfigChanged(config);
        }
    }

    //+---------------------------------------------------------------------------
    //
    // ITfSource
    //
    //----------------------------------------------------------------------------

    STDMETHODIMP AdviseSink(REFIID riid, IUnknown *unknown, DWORD *cookie) override {
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

    STDMETHODIMP UnadviseSink(DWORD dwCookie) override {
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

    STDMETHODIMP GetInfo(TF_LANGBARITEMINFO *pInfo) override {
        *pInfo = m_info;
        return S_OK;
    }

    STDMETHODIMP GetStatus(DWORD *pdwStatus) override {
        *pdwStatus = m_status;
        return S_OK;
    }

    STDMETHODIMP Show(BOOL fShow) override {
        return E_NOTIMPL;
    }

    STDMETHODIMP GetTooltipString(BSTR *pbstrToolTip) override {
        return E_NOTIMPL;
    }

    STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT *prcArea) override {
        if (click == TF_LBI_CLK_LEFT) {
            m_service->TipOnOff();
        } else {
            m_popup->Show(pt);
        }

        return S_OK;
    }

    STDMETHODIMP InitMenu(ITfMenu *pMenu) override {
        return E_NOTIMPL;
    }

    STDMETHODIMP OnMenuSelect(UINT wID) override {
        return E_NOTIMPL;
    }

    STDMETHODIMP GetIcon(HICON *phIcon) override {
        TRY_FOR_HRESULT
        using namespace proto;

        auto icon_resource = IDI_MODE_ALPHA;
        auto light_theme = Config::SystemUsesLightTheme();

        if (m_service->config()->ime_enabled().value()) {
            switch (m_service->config()->input_mode()) {
            case IM_CONTINUOUS:
                icon_resource = light_theme ? IDI_MODE_CONTINUOUS : IDI_MODE_CONTINUOUS_W;
                break;
            case IM_BASIC:
                icon_resource = light_theme ? IDI_MODE_BASIC : IDI_MODE_BASIC_W;
                break;
            case IM_PRO:
                icon_resource = light_theme ? IDI_MODE_PRO : IDI_MODE_PRO_W;
                break;
            }
        } else {
            icon_resource = light_theme ? IDI_MODE_ALPHA : IDI_MODE_ALPHA_W;
        }

        auto icon =
            ::LoadImage(m_service->hmodule(), MAKEINTRESOURCE(icon_resource), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);

        *phIcon = static_cast<HICON>(icon);

        CATCH_FOR_HRESULT;
    }

    STDMETHODIMP GetText(BSTR *pbstrText) override {
        return E_NOTIMPL;
    }

  private:
    com_ptr<TextService> m_service = nullptr;
    std::unordered_map<DWORD, com_ptr<ITfLangBarItemSink>> m_sinks;
    std::unique_ptr<PopupMenu> m_popup = nullptr;

    TF_LANGBARITEMINFO m_info;
    DWORD m_status = 0;
    InputMode m_prev_input_mode = IM_UNSPECIFIED;
};

} // namespace

com_ptr<LangBarIndicator> LangBarIndicator::Create() {
    return as_self<LangBarIndicator>(make_self<LangBarIndicatorImpl>());
}

} // namespace khiin::win32::tip
