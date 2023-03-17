#pragma once

#include <memory>
#include <string>

namespace khiin::proto {
class Command;
class Request;
class Response;
}

namespace khiin::engine {

class BufferMgr;
class Config;
class ConfigChangeListener;
class Database;
class Dictionary;
class KeyConfig;
class SyllableParser;
class Splitter;
class Trie;
class UserDictionary;

class Engine {
  public:
    virtual ~Engine() = default;
    static std::unique_ptr<Engine> Create();
    static std::unique_ptr<Engine> Create(std::string dbfile);

    virtual void SendCommand(proto::Command *command) = 0;
    virtual void SendCommand(const proto::Request *request, proto::Response *response) = 0;
    virtual void LoadDictionary(std::string const &file_path) = 0;
    virtual void LoadUserDictionary(std::string file_path) = 0;
    virtual void RegisterConfigChangedListener(ConfigChangeListener *listener) = 0;

    virtual BufferMgr *buffer_mgr() = 0;
    virtual Database *database() = 0;
    virtual Dictionary *dictionary() = 0;
    virtual UserDictionary *user_dict() = 0;
    virtual KeyConfig *keyconfig() = 0;
    virtual SyllableParser *syllable_parser() = 0;
    virtual Config *config() = 0;
};

} // namespace khiin::engine
