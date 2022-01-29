#pragma once

#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "messages.h"

namespace khiin::engine {

const std::string CONFIG_FILE = "taikey.json";
const std::string DB_FILE = "taikey.db";

class BufferMgr;
class CandidateFinder;
class Database;
class Dictionary;
class KeyConfig;
class Segmenter;
class SyllableParser;

class Engine {
  public:
    static Engine *Create();
    static Engine *Create(std::string home_dir);

    virtual void SendCommand(messages::Command *command) = 0;

    virtual BufferMgr *buffer_mgr() = 0;
    virtual CandidateFinder *candidate_finder() = 0;
    virtual Database *database() = 0;
    virtual Dictionary *dictionary() = 0;
    virtual KeyConfig *key_configuration() = 0;
    virtual Segmenter *segmenter() = 0;
    virtual SyllableParser *syllable_parser() = 0;
};

} // namespace khiin::engine
