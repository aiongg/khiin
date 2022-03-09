#pragma once

#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace khiin::proto {
class AppConfig;
class Command;
}

namespace khiin::engine {

class BufferMgr;
// class CandidateFinder;
class Database;
class Dictionary;
class KeyConfig;
class SyllableParser;
class Splitter;
class Trie;
class ConfigChangeListener;

class Engine {
  public:
    static Engine *Create();
    static Engine *Create(std::string dbfile);

    virtual void SendCommand(proto::Command *command) = 0;

    virtual void RegisterConfigChangedListener(ConfigChangeListener *listener) = 0;

    virtual BufferMgr *buffer_mgr() = 0;
    virtual Database *database() = 0;
    virtual Dictionary *dictionary() = 0;
    virtual KeyConfig *key_configuration() = 0;
    virtual SyllableParser *syllable_parser() = 0;
    virtual proto::AppConfig *config() = 0;
};

} // namespace khiin::engine
