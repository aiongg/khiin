#pragma once

namespace khiin::win32 {

class Process {
  public:
    static void OpenSettingsApp(HMODULE hmodule);
    static void StartServerApp(HMODULE hmodule);
};

}
