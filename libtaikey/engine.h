#pragma once

#include <filesystem>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "buffer_manager.h"
#include "candidates.h"
#include "config.h"
#include "db.h"
#include "errors.h"
#include "keys.h"
#include "splitter.h"
#include "trie.h"

namespace TaiKey {

namespace fs = std::filesystem;

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
    Engine();
    void reset();

    RetVal onKeyDown(char c);
    RetVal onKeyDown(KeyCode keyCode);

    EngineState getState() const;
    std::string getBuffer() const;

  private:
    typedef RetVal (Engine::*KeyHandlerFn)(KeyCode keyCode);

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

    std::unique_ptr<TKDB> database = nullptr;
    std::unique_ptr<Config> config = nullptr;
    std::unique_ptr<Splitter> splitter = nullptr;
    std::unique_ptr<Trie> trie = nullptr;
    std::unique_ptr<BufferManager> buffer = nullptr;
    std::unique_ptr<CandidateFinder> candidateFinder = nullptr;
};

} // namespace TaiKey
