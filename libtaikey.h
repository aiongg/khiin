#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <boost/locale.hpp>
#include <string>

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
    Valid, // remove later
    Editing,
    BufferByLetter,
    BufferBySegment,
    ChoosingCandidate,
};

enum class InputMode {
    Normal,
    Lomaji,
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

class TaiKeyEngine {

  public:
    TaiKeyEngine();
    void reset();

    EngineState onKeyDown(KeyCode keyCode);
    EngineState getState() const;
    std::string getBuffer() const;

  private:
    std::string _keyBuffer;
    EngineState _engineState;
    InputMode _inputMode;
    void pop_back();

    void _setEngineState(EngineState nextEngineState, KeyCode keyCode);
    void _onChangeEngineState(EngineState prev, EngineState next,
                              KeyCode keyCode);

    void _bySegmentToByLetter(KeyCode keyCode);

    void _handleEditing(KeyCode keyCode);
    void _handleChoosingCandidate(KeyCode keyCode);
    void _handleBufferByLetter(KeyCode keyCode);
    void _handleBufferBySegment(KeyCode keyCode);

    DisplayBuffer _displayBuffer;
    int _getDisplayBufferLength();
};

} // namespace TaiKey
