#include "Dictionary.h"

//#include <algorithm>
#include <unordered_map>

#include "Database.h"
#include "Engine.h"
#include "KeyConfig.h"
#include "Lomaji.h"
#include "Splitter.h"
#include "SyllableParser.h"
#include "Trie.h"
#include "messages.h"

#include <mutex>

namespace khiin::engine {

namespace {

inline bool IsHigherFrequency(TaiToken *left, TaiToken *right) {
    return left->input_id == right->input_id ? left->weight > right->weight : left->input_id < right->input_id;
}

inline void SortTokens(std::vector<TaiToken *> &tokens) {
    std::sort(tokens.begin(), tokens.end(), IsHigherFrequency);
}

template <typename T>
void SortAndDedupe(std::vector<T> &vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

class DictionaryImpl : public Dictionary {
  public:
    DictionaryImpl(Engine *engine) : engine(engine) {}

    virtual void Initialize() override {
        LoadInputsByFreq();
        LoadInputSequences();
        BuildWordTrie();
        BuildSyllableTrie();
        BuildWordSplitter();
        engine->RegisterConfigChangedListener(this);
    }

    virtual bool StartsWithWord(std::string_view query) const override {
        return word_trie->StartsWithKey(query);
    }

    virtual bool StartsWithSyllable(std::string_view query) const override {
        return syllable_trie->StartsWithKey(query);
    }

    virtual bool IsSyllablePrefix(std::string_view query) const override {
        if (query.empty()) {
            return false;
        }

        auto maybe_tone = engine->key_configuration()->CheckToneKey(query.back());

        if (Lomaji::NeedsToneDiacritic(maybe_tone)) {
            query = query.substr(0, query.size() - 1);
        }

        return syllable_trie->HasKeyOrPrefix(query);
    }

    virtual bool IsWordPrefix(std::string_view query) const override {
        return word_trie->HasKeyOrPrefix(query);
    }

    virtual bool IsWord(std::string_view query) const override {
        return word_trie->HasKey(query);
    }

    virtual std::vector<TaiToken *> WordSearch(std::string const &query) override {
        auto ret = GetOrCacheTokens(query);
        SortTokens(ret);
        return ret;
    }

    virtual std::vector<TaiToken *> Autocomplete(std::string const &query) override {
        auto words = word_trie->Autocomplete(query, 10, 5);
        auto ret = GetOrCacheTokens(words);
        SortTokens(ret);
        return ret;
    }

    virtual std::vector<TaiToken *> AllWordsFromStart(std::string const &query) override {
        auto words = std::vector<std::string>();
        word_trie->FindKeys(query, words);
        auto ret = GetOrCacheTokens(words);
        SortTokens(ret);
        return ret;
    }

    virtual std::vector<std::string> const &AllInputsByFreq() override {
        return splitter_inputs;
    }

    virtual Splitter *word_splitter() override {
        return splitter.get();
    };

    virtual void OnConfigChanged(messages::AppConfig config) override {
        // Reload with new KeyConfig
    }

  private:
    void LoadInputsByFreq() {
        m_inputs_by_freq.clear();
        engine->database()->AllWordsByFreq(m_inputs_by_freq);
    }

    void LoadInputSequences() {
        m_token_cache.clear();
        splitter_inputs.clear();
        trie_inputs.clear();
        auto parser = engine->syllable_parser();
        auto seen = std::set<std::string>();
        auto seen_fuzzy_monosyls = std::set<std::string>();

        for (auto &row : m_inputs_by_freq) {
            auto input_sequences = parser->AsInputSequences(row.input);
            for (auto &input_seq : input_sequences) {
                auto &key = input_seq.input;
                CacheId(key, row.id);
                if (input_seq.is_fuzzy_monosyllable && seen_fuzzy_monosyls.find(key) == seen_fuzzy_monosyls.end()) {
                    seen_fuzzy_monosyls.insert(key);
                    splitter_inputs.push_back(key);
                } else if (seen.find(key) == seen.end()) {
                    seen.insert(key);
                    splitter_inputs.push_back(key);
                    trie_inputs.push_back(key);
                }
            }
        }
    }

    void BuildWordTrie() {
        auto parser = engine->syllable_parser();
        word_trie = std::unique_ptr<Trie>(Trie::Create());

        for (auto &word : trie_inputs) {
            word_trie->Insert(word);
        }
    }

    void BuildSyllableTrie() {
        syllable_trie = std::unique_ptr<Trie>(Trie::Create());
        auto syllables = string_vector();
        auto parser = engine->syllable_parser();
        engine->database()->LoadSyllables(syllables);

        for (auto &syl : syllables) {
            auto keys = parser->AsInputSequences(syl);
            for (auto &key : keys) {
                syllable_trie->Insert(key.input);
            }
        }
    }

    void BuildWordSplitter() {
        splitter = std::make_unique<Splitter>(splitter_inputs);
    }

    void CacheId(std::string const &input, int input_id) {
        auto ids = m_input_ids.find(input);
        if (ids != m_input_ids.end()) {
            if (std::find(ids->second.begin(), ids->second.end(), input_id) == ids->second.end()) {
                ids->second.push_back(input_id);
            }
        } else {
            m_input_ids.insert(std::make_pair(input, std::vector<int>{input_id}));
        }
    }

    std::vector<TaiToken *> GetOrCacheTokens(std::string const &input) {
        auto ret = std::vector<TaiToken *>();

        auto ids = m_input_ids.find(input);
        if (ids == m_input_ids.end()) {
            return ret;
        }

        return GetOrCacheTokens(ids->second);
    }

    std::vector<TaiToken *> GetOrCacheTokens(std::vector<std::string> const &inputs) {
        auto ret = std::vector<TaiToken *>();
        for (auto &input : inputs) {
            auto tokens = GetOrCacheTokens(input);
            ret.insert(ret.end(), tokens.begin(), tokens.end());
        }
        SortAndDedupe(ret);
        return ret;
    }

    std::vector<TaiToken *> GetOrCacheTokens(int input_id) {
        auto ret = std::vector<TaiToken *>();

        if (auto cached = m_input_id_token_cache.find(input_id); cached != m_input_id_token_cache.end()) {
            return cached->second;
        }

        auto tokens = std::vector<TaiToken>();
        engine->database()->ConversionsByInputId(input_id, tokens);
        for (auto &token : tokens) {
            auto inserted = m_token_cache.insert(std::make_pair(token.id, std::move(token)));
            ret.push_back(&inserted.first->second);
        }

        SortAndDedupe(ret);
        return ret;
    }

    std::vector<TaiToken *> GetOrCacheTokens(std::vector<int> const &input_ids) {
        auto ret = std::vector<TaiToken *>();

        for (auto id : input_ids) {
            auto tokens = GetOrCacheTokens(id);
            ret.insert(ret.end(), tokens.begin(), tokens.end());
        }

        SortAndDedupe(ret);
        return ret;
    }

    Engine *engine = nullptr;
    std::unique_ptr<Trie> word_trie = nullptr;
    std::unique_ptr<Trie> syllable_trie = nullptr;
    std::unique_ptr<Splitter> splitter = nullptr;

    std::unordered_map<int, TaiToken> m_token_cache;
    std::unordered_map<int, std::vector<TaiToken *>> m_input_id_token_cache;
    std::unordered_map<std::string, std::vector<int>> m_input_ids;

    std::vector<InputByFreq> m_inputs_by_freq;
    std::vector<std::string> trie_inputs;
    std::vector<std::string> splitter_inputs;
};

} // namespace

Dictionary *Dictionary::Create(Engine *engine) {
    return new DictionaryImpl(engine);
}

} // namespace khiin::engine
