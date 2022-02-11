#pragma once

#include "BaseWindow.h"
#include "common.h"

namespace khiin::win32 {

extern std::wstring kCandidateWindowClassName;
extern GUID kCandidateWindowGuid;

enum class DisplayMode {
    Short,
    Long,
    Expanded,
};

using CandidateColumn = std::vector<messages::Candidate const *>;
using CandidateGrid = std::vector<CandidateColumn>;

class CandidateWindow : public BaseWindow<CandidateWindow> {
  public:
    static CandidateWindow *Create(HWND parent);

    // BaseWindow
    virtual LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override = 0;
    virtual std::wstring &class_name() const override = 0;

    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual bool Showing() = 0;
    virtual void SetCandidates(DisplayMode display_mode, CandidateGrid *candidate_grid, int focused_id,
                               size_t qs_col, bool qs_active) = 0;
    virtual void SetScreenCoordinates(RECT text_rect) = 0;
};

} // namespace khiin::win32
