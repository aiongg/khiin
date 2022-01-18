#pragma once

#include "KeyEvent.h"
#include "Action.h"

namespace Khiin {

struct TextService;

struct EditSession {
    static HRESULT handleAction(TextService *pService, ITfContext *pContext, Action action);
};

} // namespace Khiin
