#pragma once

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
#include "messages.h"
#include "splitter.h"
#include "trie.h"

namespace khiin::engine {

const std::string CONFIG_FILE = "taikey.json";
const std::string DB_FILE = "taikey.db";

enum class EngineState {
    Ready,
    Editing,
    BufferByLetter,
    BufferBySegment,
    ChoosingCandidate,
};

struct ImeDisplayData {
    std::string buffer;
    int cursor;
    std::vector<std::tuple<int, int, int>> underlines;
    int focus;
    std::vector<CandidateDisplay> candidates;
};

class Engine {
  public:
    using ResponseCallback = std::function<void(messages::Command *)>;
    // Engine();
    // Engine(std::string resourceDir);

    virtual void SendCommand(messages::Command *command) = 0;

    /*
        Allow the application to check before sending a key
        whether the Engine is currently able to consume this
        key or not. If the return value is true, the application
        should send the key to onKeyDown for processing.
    */
    // auto TestConsumable(KeyCode keyCode) -> bool;

    /*
        This is the main method applications should use to provide
        input to the taikey processing engine. KeyCodes are listed
        in keys.h, and the application must convert them to appropriate
        values before sending. Any keys not listed in keys.h are
        unable to be handled by the engine and should not be sent.

        The second parameter should be an empty ImeDisplayData object,
        which on return will contain the data required to display
        the buffer_, underline segments, colors or hint text, and
        candidate availability.
    */
    // auto onKeyDown(KeyCode kc, ImeDisplayData &data) -> RetVal;

    /*
        The application should handle candidate navigation directly,
        as the number, pagination, or other display characteristics
        are best determined by the user-facing application. As the user
        navigates through candidates_, the index in the ImeDisplayData
        canddiate list of the candidate currently under user focus
        should be passed back to this method. On return, the data
        object will contain an updated display buffer_ showing the
        focused candidate.
    */
    /*
    auto focusCandidate(size_t index, ImeDisplayData &data) -> RetVal;

    RetVal Reset();

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

    std::unique_ptr<TKDB> database = nullptr;
    std::unique_ptr<Config> config = nullptr;
    std::unique_ptr<Splitter> splitter = nullptr;
    std::unique_ptr<Trie> trie = nullptr;
    std::unique_ptr<BufferManager> buffer = nullptr;
    std::unique_ptr<CandidateFinder> candidateFinder = nullptr;
    */
};

class EngineFactory {
  public:
    static Engine *Create();
    static Engine *Create(std::string home_dir);
};

} // namespace khiin::engine
