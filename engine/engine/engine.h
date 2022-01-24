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
    virtual void SendCommand(messages::Command *command) = 0;
};

class EngineFactory {
  public:
    static Engine *Create();
    static Engine *Create(std::string home_dir);
};

} // namespace khiin::engine
