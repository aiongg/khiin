#pragma once

#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <boost/locale.hpp>

#include "keys.h"

namespace TaiKey {

static bool READY = false;
inline void initialize() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    boost::locale::generator gen;
    std::locale loc = gen("");
    std::locale::global(loc);
    std::wcout.imbue(loc);
    READY = true;
}

enum class EngineState {
    Ready,
    Editing,
    BufferByLetter,
    BufferBySegment,
    ChoosingCandidate,
};

enum class InputMode {
    Normal,
    Pro,
};

struct DisplayBufferSegment {
    int rawTextLength;
    int inputTextLength;
    int displayTextLength;
    std::string rawText;
    std::string inputText;
    std::string displayText;
};

struct DisplayBuffer {
    int segmentCount;
    int selectedSegment;
    int cursorPosition;
    int displaySegmentOffsets[20];
    DisplayBufferSegment segments[20];
};

class TKEngine {

  public:
    TKEngine();
    void reset();

    bool onKeyDown(char c);
    bool onKeyDown(KeyCode keyCode);
    EngineState getState() const;
    std::string getBuffer() const;

  private:
    typedef bool (TKEngine::*KeyHandlerFn)(KeyCode keyCode);

    std::string keyBuffer_;
    EngineState engineState_;
    InputMode inputMode_;

    void popBack_();

    bool onKeyDown_(KeyCode keyCode);
    bool onKeyDownNormal_(KeyCode keyCode);
    bool onKeyDownPro_(KeyCode keyCode);

    void setEngineState_(EngineState nextEngineState, KeyCode keyCode);
    void onChangeEngineState_(EngineState prev, EngineState next,
                              KeyCode keyCode);

    void bySegmentToByLetter_(KeyCode keyCode);

    bool handleKeyOnReady_(KeyCode keyCode);
    bool handleEditing_(KeyCode keyCode);
    bool handleChoosingCandidate_(KeyCode keyCode);
    bool handleNavByLetter_(KeyCode keyCode);
    bool handleNavBySegment_(KeyCode keyCode);

    DisplayBuffer displayBuffer_;
    int getDisplayBufferLength_();
};

} // namespace TaiKey
