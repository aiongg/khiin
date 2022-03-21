#pragma once

#include "KeyEvent.h"

namespace khiin::proto {
class Command;
}

namespace khiin::win32::tip {

struct TextService;

struct EditSession {
    static void HandleFocusChange(TextService *service, ITfDocumentMgr *docmgr);
    static void HandleAction(TextService *service, ITfContext *context, proto::Command *command);
};

} // namespace khiin::win32::tip
