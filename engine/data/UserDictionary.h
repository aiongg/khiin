#pragma once

#include <memory>
#include <string>
#include <vector>

namespace khiin::engine {

struct TokenResult;

class UserDictionary {
  public:
    UserDictionary() = default;
    UserDictionary &operator=(UserDictionary const &) = delete;
    UserDictionary(UserDictionary const &) = delete;
    virtual ~UserDictionary() = 0;
    static std::unique_ptr<UserDictionary> Create(std::string filename);
    virtual std::vector<TokenResult> Search(std::string_view query) = 0;
    virtual std::vector<TokenResult> SearchExact(std::string const &query) = 0;
    virtual bool HasExact(std::string_view query) = 0;
    virtual size_t StartsWithWord(std::string_view query) = 0;
};

} // namespace khiin::engine
