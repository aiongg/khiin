#pragma once

#include <memory>
#include <string>

namespace khiin::engine {

class UserDictionaryParser {
  public:
    virtual ~UserDictionaryParser() = default;
    static std::unique_ptr<UserDictionaryParser> LoadFile(std::string filename);
    virtual bool Advance() = 0;
    virtual std::pair<std::string, std::string> GetRow() = 0;
};

} // namespace khiin::engine
