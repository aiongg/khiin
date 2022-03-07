#pragma once

#include "BaseWindow.h"
#include "CandidatePager.h"
#include "GuiWindow.h"
#include "common.h"

namespace khiin::win32 {

extern const std::wstring kCandidateWindowClassName;
extern GUID kCandidateWindowGuid;

struct ColorScheme;

enum class DisplaySize {
    S,
    M,
    L,
    XL,
    XXL,
};

struct CandidateSelectListener {
    virtual void OnSelectCandidate(int32_t id) = 0;
};

class CandidateWindow2 : public GuiWindow {
  public:
    static CandidateWindow2 *Create(HWND parent);

    virtual void SetCandidates(DisplayMode display_mode, CandidateGrid *candidate_grid, int focused_id, size_t qs_col,
                               bool qs_active, RECT text_position) = 0;
    virtual void SetDisplaySize(int size) = 0;
    virtual void SetAppearance(ColorScheme const &color_scheme) = 0;
    virtual void RegisterCandidateSelectListener(CandidateSelectListener *listener) = 0;
};

} // namespace khiin::win32
