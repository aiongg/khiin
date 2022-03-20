#pragma once

#include <memory>
#include <string>

namespace khiin::proto {
class Command;
}

namespace khiin::engine {

class BufferMgr;
class Config;
class Database;
class Dictionary;
class KeyConfig;
class SyllableParser;
class Splitter;
class Trie;
class ConfigChangeListener;

class Engine {
  public:
    static std::unique_ptr<Engine> Create();
    static std::unique_ptr<Engine> Create(std::string dbfile);

    virtual void SendCommand(proto::Command *command) = 0;

    virtual void RegisterConfigChangedListener(ConfigChangeListener *listener) = 0;

    virtual BufferMgr *buffer_mgr() = 0;
    virtual Database *database() = 0;
    virtual Dictionary *dictionary() = 0;
    virtual KeyConfig *keyconfig() = 0;
    virtual SyllableParser *syllable_parser() = 0;
    virtual Config *config() = 0;
};

} // namespace khiin::engine