#include "data/Dictionary.h"

#include <unordered_map>

#include "config/KeyConfig.h"
#include "input/Buffer.h"
#include "input/Lomaji.h"
#include "input/SyllableParser.h"

#include "Database.h"
#include "Engine.h"
#include "Splitter.h"
#include "Trie.h"
#include "UserDictionary.h"

namespace khiin::engine {

namespace {
using namespace proto;

template <typename T>
void SortAndDedupe(std::vector<T> &vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

class DictionaryImpl : public Dictionary {
  public:
    explicit DictionaryImpl(Engine *engine) : m_engine(engine) {}

  private:
    void Initialize() override {
        LoadKeySequences();
        BuildWordTrie();
        BuildSyllableTrie();
        BuildWordSplitter();
        LoadPunctuation();
        m_engine->RegisterConfigChangedListener(this);
    }

    void Uninitialize() override {
        m_word_trie.reset(nullptr);
        m_word_splitter.reset(nullptr);
        m_syllable_trie.reset(nullptr);
        m_input_ids.clear();
        m_user_inputs.clear();
        m_token_cache.clear();
        m_input_id_token_cache.clear();
        m_key_sequences.clear();
        m_punctuation.clear();
    }

    bool StartsWithWord(std::string_view query) const override {
        return m_word_trie->StartsWithKey(query);
    }

    bool StartsWithSyllable(std::string_view query) const override {
        return m_syllable_trie->StartsWithKey(query);
    }

    bool IsSyllablePrefix(std::string_view query) const override {
        if (query.empty()) {
            return false;
        }

        auto maybe_tone = m_engine->keyconfig()->CheckToneKey(query.back());

        if (Lomaji::NeedsToneDiacritic(maybe_tone)) {
            query = query.substr(0, query.size() - 1);
        }

        return m_syllable_trie->HasKeyOrPrefix(query);
    }

    bool IsWordPrefix(std::string_view query) const override {
        return m_word_trie->HasKeyOrPrefix(query);
    }

    bool IsWord(std::string_view query) const override {
        return m_word_trie->HasKey(query);
    }

    std::vector<TaiToken> WordSearch(std::string const &query) override {
        auto ret = std::vector<TaiToken>();
        auto words = std::vector<std::string>();
        words.push_back(query);
        m_engine->database()->LoadConversions(words, InputType::Numeric, ret);
        return ret;
    }

    std::vector<TaiToken> Autocomplete(std::string const &query) override {
        auto ret = std::vector<TaiToken>();
        auto words = m_word_trie->Autocomplete(query, 10, 5); // NOLINT
        m_engine->database()->LoadConversions(words, InputType::Numeric, ret);
        return ret;
    }

    std::vector<TaiToken> AllWordsFromStart(std::string const &query) override {
        auto ret = std::vector<TaiToken>();
        auto words = std::vector<std::string>();
        auto input_size = unicode::u8_size(query);
        m_word_trie->FindKeys(query, words);
        m_engine->database()->LoadConversions(words, InputType::Numeric, ret);
        AddUserDictionaryCandidates(query, ret);
        for (auto &token : ret) {
            token.input_size = unicode::u8_size(token.key_sequence);
        }
        return ret;
    }

    void AddUserDictionaryCandidates(std::string const &query, std::vector<TaiToken> &ret) {
        auto *userdict = m_engine->user_dict();
        if (userdict == nullptr) {
            return;
        }

        auto extras = userdict->Search(query);

        if (!extras.empty()) {
            ret.insert(ret.end(), extras.begin(), extras.end());
        }
    }

    std::vector<std::string> const &AllInputsByFreq() override {
        return m_user_inputs;
    }

    std::vector<std::vector<std::string>> Segment(std::string_view query, uint32_t limit) override {
        auto ret = std::vector<std::vector<std::string>>();
        auto query_lc = unicode::copy_str_tolower(query);
        auto segmentations = m_word_trie->Multisplit(query_lc, m_word_splitter->cost_map(), limit);
        for (auto &seg : segmentations) {
            auto vec = std::vector<std::string>();
            auto start = query.begin();
            for (auto idx : seg) {
                auto end = query.begin() + idx;
                vec.push_back(std::string(start, end));
                start = end;
            }
            ret.push_back(vec);
        }
        return ret;
    }

    std::vector<Punctuation> SearchPunctuation(std::string const &query) override {
        auto ret = std::vector<Punctuation>();

        std::copy_if(std::begin(m_punctuation), std::end(m_punctuation), std::back_inserter(ret), [&](const auto &p) {
            return query == p.input;
        });

        return ret;
    }

    Splitter *word_splitter() override {
        return m_word_splitter.get();
    };

    Trie *word_trie() override {
        return m_word_trie.get();
    };

    void OnConfigChanged(Config *config) override {
        // Reload with new KeyConfig
    }

    void LoadKeySequences() {
        m_key_sequences.clear();
        m_engine->database()->AllWordsByFreq(m_key_sequences, InputType::Numeric);
    }

    void BuildWordTrie() {
        m_word_trie = Trie::Create();

        for (auto &word : m_key_sequences) {
            m_word_trie->Insert(word);
        }
    }

    void BuildSyllableTrie() {
        m_syllable_trie = std::unique_ptr<Trie>(Trie::Create());
        auto syllables = std::vector<std::string>();
        auto *parser = m_engine->syllable_parser();
        m_engine->database()->LoadSyllables(syllables);

        for (auto &syl : syllables) {
            auto keys = parser->AsInputSequences(syl);
            for (auto &key : keys) {
                m_syllable_trie->Insert(key.input);
            }
        }
    }

    void BuildWordSplitter() {
        m_word_splitter = std::make_unique<Splitter>(m_key_sequences);
    }

    void LoadPunctuation() {
        m_punctuation.clear();
        m_engine->database()->LoadPunctuation(m_punctuation);
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

    void RecordNGrams(Buffer const &buffer) override {
        if (buffer.Empty()) {
            return;
        }

        auto unigrams = std::vector<std::string>();
        auto bigrams = std::vector<std::pair<std::string, std::string>>();

        auto it = buffer.CBegin();
        auto end = buffer.CEnd();
        for (; it != end; ++it) {
            if (it[0].IsTaiText() && it[0].IsConverted()) {
                auto lgram = it[0].converted();

                unigrams.push_back(lgram);
                if (it != end - 1 && it[1].IsTaiText() && it[1].IsConverted()) {
                    bigrams.push_back({std::move(lgram), it[1].converted()});
                }
            }
        }

        m_engine->database()->RecordUnigrams(std::move(unigrams));
        m_engine->database()->RecordBigrams(std::move(bigrams));
    }

    Engine *m_engine = nullptr;
    std::unique_ptr<Trie> m_word_trie = nullptr;
    std::unique_ptr<Splitter> m_word_splitter = nullptr;
    std::unique_ptr<Trie> m_syllable_trie = nullptr;

    // Calculated; depend on KeyConfig
    std::unordered_map<std::string, std::vector<int>> m_input_ids;
    std::vector<std::string> m_user_inputs;

    // From the database
    std::unordered_map<int, TaiToken> m_token_cache;
    std::unordered_map<int, std::vector<TaiToken *>> m_input_id_token_cache;
    std::vector<std::string> m_key_sequences;
    std::vector<Punctuation> m_punctuation;
};

} // namespace

Dictionary::~Dictionary() = default;

std::unique_ptr<Dictionary> Dictionary::Create(Engine *engine) {
    return std::make_unique<DictionaryImpl>(engine);
}

} // namespace khiin::engine
