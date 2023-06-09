#pragma once

namespace khiin::proto {
class Candidate;
class CandidateList;
}

namespace khiin::win32 {

enum class DisplayMode {
    ShortColumn,
    LongColumn,
    Grid,
};

using CandidateColumn = std::vector<proto::Candidate const *>;
using CandidateGrid = std::vector<CandidateColumn>;

class CandidatePager {
  public:
    static CandidatePager *Create();

    virtual void SetCandidateList(proto::CandidateList *candidate_list) = 0;
    virtual void SetDisplayMode(DisplayMode display_mode) = 0;
    virtual void SetFocus(int candidate_id) = 0;
    virtual void SetFocusIndex(int index) = 0;

    virtual DisplayMode GetDisplayMode() = 0;
    virtual void GetPage(CandidateGrid &grid) = 0;
    virtual int GetFocusedCandidateId() = 0;
    virtual size_t GetFocusedColumnIndex() = 0;
    virtual int PageCount() = 0;
    virtual int MaxPageSize() = 0;
    virtual int CurrentPageIndex() = 0;

    virtual int NextPageCandidateId() = 0;
    virtual int PrevPageCandidateId() = 0;
};

} // namespace khiin::win32
