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

    virtual std::vector<std::string> WordSearch(std::string_view query) override {
        auto ret = std::vector<std::string>();
        word_trie->FindWords(query, ret);
        return ret;
    }

    virtual DictionaryRow *BestWord(std::string const &query) override {
        auto it = input_id_map.find(query);
        if (it == input_id_map.end()) {
            return nullptr;
        }
        auto id = it->second[0];
        auto found =
            std::find_if(dictionary_entries.begin(), dictionary_entries.end(), [&](DictionaryRow const &entry) {
                return id == entry.id;
            });
        if (found == dictionary_entries.end()) {
            return nullptr;
        }
        return &*found;
    }

    virtual DictionaryRow *BestAutocomplete(std::string const &query) override {
        auto words = word_trie->Autocomplete(query, 1);
        if (words.size()) {
            return BestWord(words[0]);
        }
        return nullptr;
    }

    virtual std::vector<std::string> const &AllInputsByFreq() override {
        return splitter_words;
    }

    virtual Splitter *word_splitter() override {
        return splitter.get();
    };

  private:
    void LoadDictionaryData() {
        dictionary_entries.clear();
        engine->database()->AllWordsByFreq(dictionary_entries);
    }

    void LoadInputSequences() {
        input_id_map.clear();
        splitter_words.clear();
        trie_words.clear();
        auto parser = engine->syllable_parser();
        auto seen = std::set<std::string>();
        auto seen_fuzzy_monosyls = std::set<std::string>();

        for (auto &entry : dictionary_entries) {
            auto inputs = parser->AsInputSequences(entry.input);

            for (auto &ea : inputs) {
                CacheInputId(ea.input, entry.id);
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

    void CacheInputId(std::string input, int id) {
        if (auto it = input_id_map.find(input); it != input_id_map.end()) {
            auto &ids = it->second;
            if (std::find(ids.cbegin(), ids.cend(), id) == ids.cend()) {
                ids.push_back(id);
            }
        } else {
            input_id_map[input] = std::vector<int>{id};
        }
    }

    Engine *engine = nullptr;
    std::unique_ptr<Trie> word_trie = nullptr;
    std::unique_ptr<Trie> syllable_trie = nullptr;
    std::unique_ptr<Splitter> splitter = nullptr;

    std::vector<DictionaryRow> dictionary_entries;
    std::vector<std::string> trie_words;
    std::vector<std::string> splitter_words;
    std::unordered_map<std::string, std::vector<int>> input_id_map;
};

} // namespace

Dictionary *Dictionary::Create(Engine *engine) {
    return new DictionaryImpl(engine);
}

} // namespace khiin::engine
