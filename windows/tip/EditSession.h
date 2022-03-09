#pragma once

#include "KeyEvent.h"

namespace khiin::proto {
class Command;
}

namespace khiin::win32 {

struct TextService;

struct EditSession {
    static void HandleAction(TextService *pService, ITfContext *pContext, proto::Command *command);
};

} // namespace khiin::win32
