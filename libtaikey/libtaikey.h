#pragma once

#include <string>

#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#endif

//#include <boost/locale.hpp>

#include "buffer_manager.h"
#include "candidates.h"
#include "config.h"
#include "db.h"
#include "errors.h"
#include "keys.h"
#include "syl_splitter.h"
#include "trie.h"

namespace TaiKey {

namespace fs = std::filesystem;

// static bool READY = false;
// inline void initialize() {
//#ifdef _WIN32
//    SetConsoleOutputCP(CP_UTF8);
//#endif
//    boost::locale::generator gen;
//    std::locale loc = gen("");
//    std::locale::global(loc);
//    std::wcout.imbue(loc);
//    READY = true;
//}

const std::string CONFIG_FILE = "taikey.json";
const std::string DB_FILE = "taikey.db";

enum class EngineState {
    Ready,
    Editing,
    BufferByLetter,
    BufferBySegment,
    ChoosingCandidate,
};

class Engine {
  public:
    Engine(std::string resourceDir);
    void reset();

    RetVal onKeyDown(char c);
    RetVal onKeyDown(KeyCode keyCode);

    EngineState getState() const;
    std::string getBuffer() const;

  private:
    typedef RetVal (Engine::*KeyHandlerFn)(KeyCode keyCode);

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

    // from newer engine version
    fs::path tkFolder_;
    TKDB database_;
    Config config_;
    Splitter splitter_;
    Trie trie_;
    std::unique_ptr<BufferManager> buffer_;
    std::unique_ptr<CandidateFinder> candidateFinder;
};

} // namespace TaiKey
