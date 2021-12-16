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

    retval_t onKeyDown(char c);
    retval_t onKeyDown(KeyCode keyCode);

    EngineState getState() const;
    std::string getBuffer() const;

  private:
    typedef retval_t (TKEngine::*KeyHandlerFn)(KeyCode keyCode);

    Buffer buffer_;
    std::string keyBuffer_;
    EngineState engineState_;
    InputMode inputMode_;
    ToneKeys toneKeys_;

    void popBack_();

    retval_t onKeyDown_(KeyCode keyCode);
    retval_t onKeyDownNormal_(KeyCode keyCode);
    retval_t onKeyDownPro_(KeyCode keyCode);

    retval_t setEngineState_(EngineState nextEngineState, KeyCode keyCode);
    retval_t handleStateTransition_(EngineState prev, EngineState next,
                              KeyCode keyCode);

    retval_t bySegmentToByLetter_(KeyCode keyCode);

    retval_t handleKeyOnReady_(KeyCode keyCode);
    retval_t handleEditing_(KeyCode keyCode);
    retval_t handleChoosingCandidate_(KeyCode keyCode);
    retval_t handleNavByLetter_(KeyCode keyCode);
    retval_t handleNavBySegment_(KeyCode keyCode);

    int getDisplayBufferLength_();
};

} // namespace TaiKey
