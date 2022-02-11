#include "pch.h"

#include "CandidateListUI.h"

#include "CandidateWindow.h"
#include "EditSession.h"
#include "TextService.h"
#include "Utils.h"
#include "common.h"

namespace khiin::win32 {
namespace {

using namespace messages;

constexpr size_t kExpandedCols = 4;
constexpr size_t kShortColSize = 5;
constexpr size_t kLongColSize = 9;

static inline auto divide_ceil(unsigned int x, unsigned int y) {
    return x / y + (x % y != 0);
}

class CandidatePager {
  public:
    CandidatePager(CandidateList *const list) : m_candidate_list(list) {}

    void SetDisplayMode(DisplayMode mode) {
        display_mode = mode;
    }

    void SetFocus(int id) {
        auto index = 0;

        for (auto &candidate : m_candidate_list->candidates()) {
            if (id == candidate.id()) {
                m_focused_index = index;
                break;
            }

            ++index;
        }
    }

    void GetPage(CandidateGrid &grid, size_t &focused_col) {
        auto &candidates = m_candidate_list->candidates();

        if (candidates.empty()) {
            return;
        }

        auto total = candidates.size();
        auto max_cols_per_page = (display_mode == DisplayMode::Expanded) ? kExpandedCols : 1;
        auto max_col_size = display_mode == DisplayMode::Short ? kShortColSize : kLongColSize;
        auto max_page_size = max_cols_per_page * max_col_size;

        auto curr_page = std::div(static_cast<int>(m_focused_index), max_page_size).quot;
        focused_col = std::div(static_cast<int>(m_focused_index), max_col_size).quot;

        D("curr_page: ", curr_page);

        auto start_index = max_page_size * curr_page;
        auto end_index = min(total, max_page_size * (curr_page + 1));
        auto n_cols = divide_ceil(end_index - start_index, max_col_size);

        {
            auto start = candidates.begin() + start_index;
            auto it = start;
            auto end = candidates.begin() + end_index;
            auto col = CandidateColumn();
            for (; it != end; ++it) {
                if (it == start + max_col_size) {
                    grid.push_back(std::move(col));
                    col = CandidateColumn();
                    start = it;
                }

                col.push_back(&*it);
            }
            grid.push_back(std::move(col));
        }
    }

  private:
    CandidateList *const m_candidate_list;
    DisplayMode display_mode = DisplayMode::Short;
    size_t m_focused_index = 0;
};

struct CandidateListUIImpl :
    public winrt::implements<CandidateListUIImpl, ITfCandidateListUIElementBehavior,
                             ITfIntegratableCandidateListUIElement, CandidateListUI> {
    virtual void Initialize(TextService *pTextService) override {
        service.copy_from(pTextService);
    }

    virtual void Uninitialize() override {
        D(__FUNCTIONW__);
        service = nullptr;
        context = nullptr;
        DestroyCandidateWindow();
        m_candidate_window.reset(nullptr);
    }

    virtual void DestroyCandidateWindow() override {
        D(__FUNCTIONW__);
        if (m_candidate_window && m_candidate_window->hwnd()) {
            ::DestroyWindow(m_candidate_window->hwnd());
        }
    }

    virtual void Update(ITfContext *pContext, EditState edit_state, const messages::CandidateList &candidate_list,
                        RECT text_rect) override {
        D(__FUNCTIONW__);
        context.copy_from(pContext);
        m_candidate_list.CopyFrom(candidate_list);

        if (!m_candidate_window) {
            makeCandidateWindow();
        }

        auto focused_id = -1;
        auto display_mode = DisplayMode::Short;
        size_t focused_col = 0;
        bool qs_active = false;

        if (edit_state == EditState::EDIT_COMPOSING) {
            m_pager = std::make_unique<CandidatePager>(&m_candidate_list);
            m_edit_state = edit_state;
        } else if ( edit_state == EditState::EDIT_SELECTING) {
            if (m_edit_state != EditState::EDIT_SELECTING) {
                m_pager = std::make_unique<CandidatePager>(&m_candidate_list);
            }

            if (m_candidate_list.focused() >= kShortColSize) {
                display_mode = DisplayMode::Long;
            }

            focused_id = m_candidate_list.focused();
            m_edit_state = edit_state;
            qs_active = true;
        }

        D("EditState: ", m_edit_state, ", DisplayMode: ", static_cast<int>(display_mode));

        m_pager->SetDisplayMode(display_mode);
        m_pager->SetFocus(focused_id);
        m_candidate_grid.clear();
        m_pager->GetPage(m_candidate_grid, focused_col);
        m_candidate_window->SetScreenCoordinates(text_rect);
        m_candidate_window->SetCandidates(display_mode, &m_candidate_grid, focused_id, focused_col, qs_active);
        Show();
    }

    virtual bool Showing() override {
        return m_candidate_window && m_candidate_window->Showing();
    }

    virtual void Show() override {
        if (m_candidate_window) {
            m_candidate_window->Show();
        }
    }

    virtual void Hide() override {
        if (m_candidate_window) {
            m_candidate_window->Hide();
        }
    }

    //+---------------------------------------------------------------------------
    //
    // ITfUIElement
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP GetDescription(BSTR *pbstrDescription) override {
        TRY_FOR_HRESULT;
        BSTR bstr = ::SysAllocString(kCandidateWindowClassName.data());
        pbstrDescription = &bstr;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP GetGUID(GUID *pguid) override {
        TRY_FOR_HRESULT;
        *pguid = kCandidateWindowGuid;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP Show(BOOL bShow) override {
        TRY_FOR_HRESULT;
        if (!m_candidate_window) {
            return S_OK;
        }

        if (bShow) {
            Show();
        } else {
            Hide();
        }

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP IsShown(BOOL *pbShow) override {
        D(__FUNCTIONW__);
        TRY_FOR_HRESULT;
        if (!m_candidate_window) {
            *pbShow = FALSE;
            return S_OK;
        }

        *pbShow = Showing() ? TRUE : FALSE;
        CATCH_FOR_HRESULT;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCandidateListUIElement
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP GetUpdatedFlags(DWORD *pdwFlags) override {
        return E_NOTIMPL;
    }
    virtual STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppdim) override {
        return E_NOTIMPL;
    }
    virtual STDMETHODIMP GetCount(UINT *puCount) override {
        return E_NOTIMPL;
    }
    virtual STDMETHODIMP GetSelection(UINT *puIndex) override {
        return E_NOTIMPL;
    }
    virtual STDMETHODIMP GetString(UINT uIndex, BSTR *pstr) override {
        return E_NOTIMPL;
    }
    virtual STDMETHODIMP GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt) override {
        return E_NOTIMPL;
    }
    virtual STDMETHODIMP SetPageIndex(UINT *pIndex, UINT uPageCnt) override {
        return E_NOTIMPL;
    }
    virtual STDMETHODIMP GetCurrentPage(UINT *puPage) override {
        return E_NOTIMPL;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfCandidateListUIElementBehavior
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP SetSelection(UINT nIndex) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP Finalize(void) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP Abort(void) override {
        return E_NOTIMPL;
    }

    //+---------------------------------------------------------------------------
    //
    // ITfIntegratableCandidateListUIElement
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP SetIntegrationStyle(GUID guidIntegrationStyle) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP GetSelectionStyle(TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP ShowCandidateNumbers(BOOL *pfShow) override {
        return E_NOTIMPL;
    }

    virtual STDMETHODIMP FinalizeExactCompositionString(void) override {
        return E_NOTIMPL;
    }

  private:
    void makeCandidateWindow() {
        auto contextView = winrt::com_ptr<ITfContextView>();
        winrt::check_hresult(context->GetActiveView(contextView.put()));

        HWND parentWnd;
        winrt::check_hresult(contextView->GetWnd(&parentWnd));

        if (!(parentWnd)) {
            parentWnd = ::GetFocus();
        }

        m_candidate_window = std::unique_ptr<CandidateWindow>(CandidateWindow::Create(parentWnd));
    }

    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
    CandidateList m_candidate_list = {};
    std::unique_ptr<CandidateWindow> m_candidate_window = nullptr;
    std::unique_ptr<CandidatePager> m_pager = nullptr;
    EditState m_edit_state = EditState::EDIT_EMPTY;
    CandidateGrid m_candidate_grid = CandidateGrid();
};

} // namespace

HRESULT CandidateListUIFactory::Create(CandidateListUI **ppCandidateListUI) {
    as_self<CandidateListUI>(winrt::make_self<CandidateListUIImpl>()).copy_to(ppCandidateListUI);
    return S_OK;
}

} // namespace khiin::win32
