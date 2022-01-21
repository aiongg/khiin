#pragma once

#include "KeyEvent.h"
#include "Action.h"

namespace khiin::win32 {

struct TextService;

struct EditSession {
    static void HandleAction(TextService *pService, ITfContext *pContext, Action action);
};

} // namespace khiin::win32
