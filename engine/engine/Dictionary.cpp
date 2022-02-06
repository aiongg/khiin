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

static auto empty_dictionary_row = DictionaryRow{};
size_t kDictionarySize = 17000;
size_t kDistinctInputs = kDictionarySize * 2;

class DictionaryImpl : public Dictionary {
  public:
    DictionaryImpl(Engine *engine) : engine(engine) {}

    virtual void Initialize() override {
        dictionary_entries.reserve(kDictionarySize);
        splitter_words.reserve(kDistinctInputs);
        trie_words.reserve(kDistinctInputs);
        LoadDictionaryData();
        LoadInputSequences();
        BuildWordTrie();
        BuildSyllableTrie();
        BuildWordSplitter();
        engine->RegisterConfigChangedListener(this);
    }

    virtual bool StartsWithWord(std::string_view query) override {
        return word_trie->StartsWithKey(query);
    }

    virtual bool StartsWithSyllable(std::string_view query) override {
        return syllable_trie->StartsWithKey(query);
    }

    virtual bool IsSyllablePrefix(std::string_view query) override {
        if (query.empty()) {
            return false;
        }

        auto maybe_tone = engine->key_configuration()->CheckToneKey(query.back());

        if (Lomaji::NeedsToneDiacritic(maybe_tone)) {
            query = query.substr(0, query.size() - 1);
        }

        return syllable_trie->HasKeyOrPrefix(query);
    }

    virtual bool IsWordPrefix(std::string_view query) override {
        return word_trie->HasKeyOrPrefix(query);
    }

    virtual bool IsWord(std::string_view query) override {
        return word_trie->HasKey(query);
    }

    virtual std::vector<DictionaryRow *> WordSearch(std::string const &query) override {
        for (auto &[key, entries] : input_entry_map) {
            if (key == query) {
                return entries;
            }
        }

        return std::vector<DictionaryRow *>();
    }

    virtual std::vector<DictionaryRow *> Autocomplete(std::string const &query) override {
        auto ret = std::vector<DictionaryRow *>();
        auto words = word_trie->Autocomplete(query, 10, 5);

        if (words.empty()) {
            return ret;
        }

        auto entries = std::set<DictionaryRow *>();
        for (auto &[key, val] : input_entry_map) {
            if (auto it = std::find(words.begin(), words.end(), key); it != words.end()) {
                for (auto &entry : val) {
                    entries.insert(entry);
                }
                words.erase(it);
            }
            if (words.empty()) {
                break;
            }
        }

        ret.assign(entries.begin(), entries.end());
        return ret;
    }

    virtual std::vector<std::string> const &AllInputsByFreq() override {
        return splitter_words;
    }

    virtual Splitter *word_splitter() override {
        return splitter.get();
    };

    virtual void OnConfigChanged(messages::AppConfig config) override {
        // Reload with new KeyConfig
    }

  private:
    void LoadDictionaryData() {
        dictionary_entries.clear();
        engine->database()->AllWordsByFreq(dictionary_entries);
    }

    void LoadInputSequences() {
        input_entry_map.clear();
        splitter_words.clear();
        trie_words.clear();
        auto parser = engine->syllable_parser();
        auto seen = std::set<std::string>();
        auto seen_fuzzy_monosyls = std::set<std::string>();

        for (auto &entry : dictionary_entries) {
            auto inputs = parser->AsInputSequences(entry.input);

            for (auto &ea : inputs) {
                CacheInput(ea.input, &entry);
                auto &key = ea.input;
                if (ea.is_fuzzy_monosyllable && seen_fuzzy_monosyls.find(key) == seen_fuzzy_monosyls.end()) {
                    seen_fuzzy_monosyls.insert(key);
                    splitter_words.push_back(key);
                } else if (seen.find(key) == seen.end()) {
                    seen.insert(key);
                    splitter_words.push_back(key);
                    trie_words.push_back(key);
                }
            }
        };
    }

    void BuildWordTrie() {
        auto parser = engine->syllable_parser();
        word_trie = std::unique_ptr<Trie>(Trie::Create());

        for (auto &word : trie_words) {
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
        splitter = std::make_unique<Splitter>(splitter_words);
    }

    void CacheInput(std::string input, DictionaryRow *entry) {
        if (auto it = input_entry_map.find(input); it != input_entry_map.end()) {
            auto &entries = it->second;
            if (std::find(entries.cbegin(), entries.cend(), entry) == entries.cend()) {
                entries.push_back(entry);
            }
        } else {
            input_entry_map[input] = std::vector<DictionaryRow *>{entry};
        }
    }

    Engine *engine = nullptr;
    std::unique_ptr<Trie> word_trie = nullptr;
    std::unique_ptr<Trie> syllable_trie = nullptr;
    std::unique_ptr<Splitter> splitter = nullptr;

    std::vector<DictionaryRow> dictionary_entries;
    std::vector<std::string> trie_words;
    std::vector<std::string> splitter_words;
    std::unordered_map<std::string, std::vector<DictionaryRow *>> input_entry_map;
};

} // namespace

Dictionary *Dictionary::Create(Engine *engine) {
    return new DictionaryImpl(engine);
}

} // namespace khiin::engine
