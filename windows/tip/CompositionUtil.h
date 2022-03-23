#pragma once

namespace khiin::proto {
class Preedit;
} // namespace khiin::proto

namespace khiin::win32::tip {

enum class SegmentStatus {
    Unmarked,
    Composing,
    Converted,
    Focused,
};

struct WidePreedit {
    int caret = 0;
    int display_size = 0;
    std::wstring preedit_display;
    std::vector<std::pair<int, int>> segment_start_and_size;
    std::vector<SegmentStatus> segment_status;
};

struct CompositionMgr;

class CompositionUtil {
  public:
    static winrt::com_ptr<ITfCompositionView> CompositionView(TfEditCookie cookie, ITfContext *context);
    static winrt::com_ptr<ITfRange> CompositionRange(TfEditCookie cookie, ITfContext *context);
    static winrt::com_ptr<ITfRange> DefaultSelectionRange(TfEditCookie cookie, ITfContext *context);
    static std::vector<InputScope> InputScopes(TfEditCookie cookie, ITfContext *context);
    static RECT TextPosition(TfEditCookie cookie, ITfContext *context, int caret);
    static std::wstring TextFromRange(TfEditCookie cookie, ITfRange *range);
    static WidePreedit const WidenPreedit(const proto::Preedit &preedit);
};

} // namespace khiin::win32::tip
