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

constexpr uint32_t kExpandedCols = 4;
constexpr uint32_t kShortColSize = 5;
constexpr uint32_t kLongColSize = 9;

static inline auto divide_ceil(unsigned int x, unsigned int y) {
    return x / y + (x % y != 0);
}

class CandidatePager {
  public:
    CandidatePager(CandidateList *const list) : m_candidate_list(list) {}

    void SetDisplayMode(DisplayMode mode) {
        if (display_mode == DisplayMode::LongColumn && mode == DisplayMode::ShortColumn) {
            return;
        }

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

        auto total = static_cast<uint32_t>(candidates.size());
        auto max_cols_per_page = (display_mode == DisplayMode::Grid) ? kExpandedCols : 1;
        auto max_col_size = display_mode == DisplayMode::ShortColumn ? kShortColSize : kLongColSize;
        auto max_page_size = max_cols_per_page * max_col_size;

        auto curr_page = std::div(static_cast<int>(m_focused_index), max_page_size).quot;

        D("curr_page: ", curr_page);

        auto start_index = max_page_size * curr_page;
        auto end_index = min(total, max_page_size * (curr_page + 1));
        auto n_cols = divide_ceil(end_index - start_index, max_col_size);
        focused_col = std::div(static_cast<int>(m_focused_index - start_index), max_col_size).quot;

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
    DisplayMode display_mode = DisplayMode::ShortColumn;
    size_t m_focused_index = 0;
};

struct CandidateListUIImpl :
    public winrt::implements<CandidateListUIImpl, ITfCandidateListUIElementBehavior,
                             ITfIntegratableCandidateListUIElement, CandidateListUI>,
    CandidateSelectListener {
    virtual void Initialize(TextService *pTextService) override {
        m_service.copy_from(pTextService);
    }

    virtual void Uninitialize() override {
        D(__FUNCTIONW__);
        m_service = nullptr;
        m_context = nullptr;
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
        m_context.copy_from(pContext);
        m_candidate_list.CopyFrom(candidate_list);

        if (!m_candidate_window) {
            makeCandidateWindow();
        }

        auto focused_id = -1;
        auto display_mode = DisplayMode::ShortColumn;
        size_t focused_col = 0;
        bool qs_active = false;

        if (edit_state == EditState::EDIT_COMPOSING) {
            m_pager = std::make_unique<CandidatePager>(&m_candidate_list);
            m_edit_state = edit_state;
        } else if (edit_state == EditState::EDIT_SELECTING) {
            if (m_edit_state != EditState::EDIT_SELECTING) {
                m_pager = std::make_unique<CandidatePager>(&m_candidate_list);
            }

            if (m_candidate_list.focused() >= kShortColSize) {
                display_mode = DisplayMode::LongColumn;
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
        D("Quickselect col: ", focused_col);
        m_candidate_window->SetCandidates(display_mode, &m_candidate_grid, focused_id, focused_col, qs_active,
                                          text_rect);
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

    virtual void OnSelectCandidate(int32_t id) override {
        m_service->OnCandidateSelected(id);
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
        winrt::check_hresult(m_context->GetActiveView(contextView.put()));

        HWND parentWnd;
        winrt::check_hresult(contextView->GetWnd(&parentWnd));

        if (!(parentWnd)) {
            parentWnd = ::GetFocus();
        }

        m_candidate_window = std::unique_ptr<CandidateWindow>(CandidateWindow::Create(parentWnd));
        m_candidate_window->RegisterCandidateSelectListener(this);
    }

    winrt::com_ptr<TextService> m_service = nullptr;
    winrt::com_ptr<ITfContext> m_context = nullptr;
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
