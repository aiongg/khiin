#pragma once

#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

//#include <boost/locale.hpp>

#include "buffer.h"
#include "errors.h"
#include "keys.h"

namespace TaiKey {

//static bool READY = false;
//inline void initialize() {
//#ifdef _WIN32
//    SetConsoleOutputCP(CP_UTF8);
//#endif
//    boost::locale::generator gen;
//    std::locale loc = gen("");
//    std::locale::global(loc);
//    std::wcout.imbue(loc);
//    READY = true;
//}

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

class TKEngine {

  public:
    TKEngine();
    void reset();

    RetVal onKeyDown(char c);
    RetVal onKeyDown(KeyCode keyCode);

    EngineState getState() const;
    std::string getBuffer() const;

  private:
    typedef RetVal (TKEngine::*KeyHandlerFn)(KeyCode keyCode);

    Buffer buffer_;
    std::string keyBuffer_;
    EngineState engineState_;
    InputMode inputMode_;
    ToneKeys toneKeys_;

    void popBack_();

    RetVal onKeyDown_(KeyCode keyCode);
    RetVal onKeyDownNormal_(KeyCode keyCode);
    RetVal onKeyDownPro_(KeyCode keyCode);

    RetVal setEngineState_(EngineState nextEngineState, KeyCode keyCode);
    RetVal handleStateTransition_(EngineState prev, EngineState next,
                              KeyCode keyCode);

    RetVal bySegmentToByLetter_(KeyCode keyCode);

    RetVal handleKeyOnReady_(KeyCode keyCode);
    RetVal handleEditing_(KeyCode keyCode);
    RetVal handleChoosingCandidate_(KeyCode keyCode);
    RetVal handleNavByLetter_(KeyCode keyCode);
    RetVal handleNavBySegment_(KeyCode keyCode);

    int getDisplayBufferLength_();
};

} // namespace TaiKey
