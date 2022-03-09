#include "pch.h"

#include "CandidatePager.h"

#include "proto/proto.h"

#include "CandidateWindow.h"

namespace khiin::win32 {
namespace {
using namespace proto;

static inline auto divide_ceil(unsigned int x, unsigned int y) {
    return x / y + (x % y != 0);
}

inline constexpr uint32_t kExpandedCols = 4;
inline constexpr uint32_t kShortColSize = 5;
inline constexpr uint32_t kLongColSize = 9;

class CandidatePagerImpl : public CandidatePager {
  public:
    virtual void SetCandidateList(CandidateList *candidate_list) {
        Reset();
        m_candidate_list = candidate_list;
    }

    virtual void SetDisplayMode(DisplayMode mode) override {
        if (m_display_mode == DisplayMode::LongColumn && mode == DisplayMode::ShortColumn) {
            return;
        }

        m_display_mode = mode;
    }

    virtual void SetFocus(int id) override {
        if (!m_candidate_list) {
            return;
        }

        m_focused_id = id;
        auto index = 0;

        for (auto &candidate : m_candidate_list->candidates()) {
            if (id == candidate.id()) {
                m_focused_index = index;
                break;
            }

            ++index;
        }

        if (index >= kShortColSize) {
            m_display_mode = DisplayMode::LongColumn;
        }
    }

    virtual DisplayMode GetDisplayMode() override {
        return m_display_mode;
    }

    virtual int GetFocusedCandidateId() override {
        return m_focused_id;
    }

    virtual size_t GetFocusedColumnIndex() override {
        return std::div(static_cast<int>(m_focused_index) - StartCandidateIndex(), MaxColSize()).quot;
    }

    virtual void GetPage(CandidateGrid &grid) override {
        if (!m_candidate_list) {
            return;
        }

        auto &candidates = m_candidate_list->candidates();

        if (candidates.empty()) {
            return;
        }

        auto max_col_size = MaxColSize();
        auto start_index = StartCandidateIndex();
        auto end_index = EndCandidateIndex();
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

    virtual int PageCount() override {
        return divide_ceil(CandidatesSize(), MaxPageSize());
    }

    virtual int MaxPageSize() override {
        return MaxColsPerPage() * MaxColSize();
    }

    virtual int CurrentPageIndex() override {
        return std::div(static_cast<int>(m_focused_index), MaxPageSize()).quot;
    }

    virtual int NextPageCandidateId() override {
        auto size = CandidatesSize();
        auto col_size = MaxColSize();
        auto next_idx = static_cast<int>(m_focused_index) + col_size;

        // go to the same candidate in the next column
        if (next_idx < size) {
            return CandidateIdAtIndex(next_idx);
        }

        // go to the last candidate
        if (next_idx < TotalColumnCount() * col_size) {
            return CandidateIdAtIndex(size - 1);
        }

        // wrap to the start
        return CandidateIdAtIndex(next_idx % col_size);
    }

    virtual int PrevPageCandidateId() override {
        auto col_size = MaxColSize();
        auto curr_idx = static_cast<int>(m_focused_index);
        auto prev_idx = curr_idx - col_size;

        if (prev_idx >= 0) {
            return CandidateIdAtIndex(prev_idx);
        }

        auto size = CandidatesSize();
        prev_idx = TotalColumnCount() * col_size - (col_size - curr_idx);

        if (prev_idx < size) {
            return CandidateIdAtIndex(prev_idx);
        }

        return CandidateIdAtIndex(size - 1);
    }

  private:
    void Reset() {
        m_display_mode = DisplayMode::ShortColumn;
        m_focused_id = -1;
        m_focused_index = 0;
        m_focused_col = 0;
    }

    int CandidateIdAtIndex(int index) {
        if (index < m_candidate_list->candidates_size()) {
            return m_candidate_list->candidates().at(index).id();
        }
        return -1;
    }

    inline int MaxColsPerPage() {
        return m_display_mode == DisplayMode::Grid ? kExpandedCols : 1;
    }

    inline int MaxColSize() {
        return m_display_mode == DisplayMode::ShortColumn ? kShortColSize : kLongColSize;
    }

    inline int CurrentPage() {
        return std::div(static_cast<int>(m_focused_index), MaxPageSize()).quot;
    }

    inline int CurrentCol() {
        return std::div(static_cast<int>(m_focused_index), MaxColSize()).quot;
    }

    inline int TotalColumnCount() {
        return divide_ceil(CandidatesSize(), MaxColSize());
    }

    inline int StartCandidateIndex() {
        return MaxPageSize() * CurrentPage();
    }

    inline int EndCandidateIndex() {
        return min(CandidatesSize(), MaxPageSize() * (CurrentPage() + 1));
    }

    inline int CandidatesSize() {
        if (m_candidate_list) {
            return m_candidate_list->candidates_size();
        }

        return 0;
    }

    CandidateList *m_candidate_list = nullptr;
    DisplayMode m_display_mode = DisplayMode::ShortColumn;
    int m_focused_id = -1;
    size_t m_focused_index = 0;
    size_t m_focused_col = 0;
};

} // namespace

CandidatePager *CandidatePager::Create() {
    return new CandidatePagerImpl();
}

} // namespace khiin::win32
