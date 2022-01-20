#pragma once

#include "KeyEvent.h"
#include "Action.h"

namespace Khiin {

struct TextService;

struct EditSession {
    static void HandleAction(TextService *pService, ITfContext *pContext, Action action);
};

} // namespace Khiin
