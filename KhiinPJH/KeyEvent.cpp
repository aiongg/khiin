#include "pch.h"

#include "KeyEvent.h"

namespace Khiin {

KeyEvent::KeyEvent(UINT message, WPARAM wParam, LPARAM lParam) : message(message), wParam(wParam), lParam(lParam) {
    if (!::GetKeyboardState(keyboardState)) {
        ::memset(keyboardState, 0, KEYBOARD_SIZE);
    }
    scanCode = LOBYTE(HIWORD(lParam));
    WORD lpChar[2];
    BYTE vkControlTmp = keyboardState[VK_CONTROL];
    keyboardState[VK_CONTROL] = 0;
    if (::ToAscii(wParam, scanCode, keyboardState, lpChar, 0) == 1) {
        ascii_ = (char)lpChar[0];
    }
    keyboardState[VK_CONTROL] = vkControlTmp;
}

char KeyEvent::ascii() {
    return ascii_;
}

} // namespace Khiin
