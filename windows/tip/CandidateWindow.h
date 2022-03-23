#pragma once

#include "BaseWindow.h"
#include "CandidatePager.h"
#include "GuiWindow.h"

namespace khiin::win32 {

extern const std::wstring kCandidateWindowClassName;

struct ColorScheme;
enum class DisplaySize;

struct CandidateSelectListener {
    virtual void OnSelectCandidate(int32_t id) = 0;
};

class CandidateWindow : public GuiWindow {
  public:
    static CandidateWindow *Create(HWND parent);

    virtual void SetCandidates(DisplayMode display_mode, CandidateGrid *candidate_grid, int focused_id, size_t qs_col,
                               bool qs_active, RECT text_position) = 0;
    virtual void RegisterCandidateSelectListener(CandidateSelectListener *listener) = 0;
    virtual void Move(RECT rect) = 0;
};

} // namespace khiin::win32
