#pragma once

namespace khiin::win32::tip {

struct CompositionMgr;

class CompositionUtil {
  public:
    static winrt::com_ptr<ITfCompositionView> CompositionView(TfEditCookie cookie, ITfContext *context);
    static winrt::com_ptr<ITfRange> DefaultSelectionRange(TfEditCookie cookie, ITfContext *context);
    static std::vector<InputScope> InputScopes(TfEditCookie cookie, ITfContext *context);
    static RECT TextPosition(TfEditCookie cookie, ITfContext *context);
};

} // namespace khiin::win32::tip
