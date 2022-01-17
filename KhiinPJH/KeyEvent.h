#pragma once

namespace Khiin {

constexpr int KEYBOARD_SIZE = 256;

class KeyEvent {
  public:
    KeyEvent() = delete;
    KeyEvent(const KeyEvent &) = default;
    KeyEvent &operator=(const KeyEvent &) = default;
    KeyEvent(KeyEvent &&) = default;
    KeyEvent &operator=(KeyEvent &&) = default;
    KeyEvent(UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    ~KeyEvent() = default;

    char ascii();

  private:
    BYTE keyboardState[KEYBOARD_SIZE];
    UINT message = 0;
    WPARAM wParam = 0;
    LPARAM lParam = 0;
    BYTE scanCode = 0;
    char ascii_ = 0;
};

} // namespace Khiin
