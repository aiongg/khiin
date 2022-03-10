#pragma once

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "TextService.h"
#include "KeyEvent.h"

namespace khiin::win32::tip { 

struct KeyEventSink : winrt::implements<KeyEventSink, IUnknown> {
    KeyEventSink() = default;
    KeyEventSink(const KeyEventSink &) = delete;
    KeyEventSink &operator=(const KeyEventSink &) = delete;
    ~KeyEventSink() = default;

    static winrt::com_ptr<KeyEventSink> Create();
    virtual void Activate(TextService *pTextService) = 0;
    virtual void Deactivate() = 0;
};

} // namespace khiin::win32
