#pragma once

namespace khiin::proto {
class Command;
}

namespace khiin::win32::tip {

struct TextService;

struct EditSession {
    using CallbackFn = std::function<void(TfEditCookie)>;

    static void HandleFocusChange(TextService *service, ITfContext *context);
    static void HandleAction(TextService *service, ITfContext *context, proto::Command *command);
    static void ReadWriteSync(TextService *service, ITfContext *context, CallbackFn callback);
};

} // namespace khiin::win32::tip
