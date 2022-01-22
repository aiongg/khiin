#include "pch.h"

#include "KeyEvent.h"

namespace khiin::win32 {

KeyEvent::KeyEvent(UINT message, WPARAM wParam, LPARAM lParam) noexcept :
    message(message), wParam(wParam), lParam(lParam) {
    if (!::GetKeyboardState(keyboardState)) {
        ::memset(keyboardState, 0, KEYBOARD_SIZE);
    }
    scanCode = LOBYTE(HIWORD(lParam));
    WORD lpChar[2];
    BYTE vkControlTmp = keyboardState[VK_CONTROL];
    keyboardState[VK_CONTROL] = 0;
    if (::ToAscii(static_cast<UINT>(wParam), scanCode, keyboardState, lpChar, 0) == 1) {
        ascii_ = (char)lpChar[0];
    }
    keyboardState[VK_CONTROL] = vkControlTmp;
}

char KeyEvent::ascii() {
    return ascii_;
}

int KeyEvent::keyCode() {
    return wParam;
}

} // namespace khiin::win32
