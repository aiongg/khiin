#pragma once

#include "KeyEvent.h"
#include "common.h"

namespace khiin::win32 {

struct TextService;

struct EditSession {
    static void HandleAction(TextService *pService, ITfContext *pContext, messages::Command &&command);
};

} // namespace khiin::win32
