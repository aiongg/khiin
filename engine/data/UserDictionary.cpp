#include "UserDictionary.h"

#include <unordered_map>

#include "Models.h"
#include "Trie.h"
#include "UserDictionaryParser.h"

namespace khiin::engine {
namespace {

using UserDictionaryEntry = std::pair<std::string, std::string>;
using UserDictionaryCache = std::unordered_map<std::string, std::vector<TokenResult>>;

class UserDictionaryImpl : public UserDictionary {
  public:
    void LoadFromFile(std::string filename) {
        auto parser = UserDictionaryParser::LoadFile(filename);
        m_trie = Trie::Create();
        while (parser->Advance()) {
            auto entry = parser->GetRow();
            m_trie->Insert(entry.first);
            m_entries.push_back(std::move(entry));
        }
    }

    std::vector<TokenResult> Search(std::string_view query) override {
        auto ret = std::vector<TokenResult>();
        auto words = std::vector<std::string>();
        m_trie->FindKeys(query, words);

        for (auto const &word : words) {
            GetOrCache(word, ret);
        }

        return ret;
    }

    std::vector<TokenResult> SearchExact(std::string const & query) override {
        auto str = std::string(query);
        auto ret = std::vector<TokenResult>();
        GetOrCache(str, ret);
        return ret;
    }

    size_t StartsWithWord(std::string_view query) override {
        return m_trie->LongestKeyOf(query);
    }

    void GetOrCache(std::string const &query, std::vector<TokenResult> &result) {
        if (auto it = m_cache.find(query); it != m_cache.end()) {
            result.insert(result.end(), it->second.begin(), it->second.end());
            return;
        }

        auto cache = std::vector<TokenResult>();
        auto end = m_entries.end();
        for (auto it = m_entries.begin(); it != end; ++it) {
            if (it->first == query) {
                m_token_cache.push_back(TaiToken());
                auto &token = m_token_cache.back();
                auto token_result = TokenResult();
                token.input = it->first;
                token.output = it->second;
                token.weight = 1000;
                token_result.token = &token;
                token_result.input_size = query.size();
                cache.push_back(std::move(token_result));
            }
        }
        if (!cache.empty()) {
            result.insert(result.end(), cache.begin(), cache.end());
        }

        m_cache.insert(std::make_pair(query, std::move(cache)));
    }

    std::vector<UserDictionaryEntry> m_entries;
    std::list<TaiToken> m_token_cache;
    UserDictionaryCache m_cache;
    std::unique_ptr<Trie> m_trie = nullptr;
};

} // namespace

UserDictionary::~UserDictionary() {}

std::unique_ptr<UserDictionary> UserDictionary::Create(std::string filename) {
    auto impl = std::make_unique<UserDictionaryImpl>();
    impl->LoadFromFile(filename);
    return impl;
}

} // namespace khiin::engine