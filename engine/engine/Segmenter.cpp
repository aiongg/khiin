#include "Segmenter.h"

#include "CandidateFinder.h"
#include "Dictionary.h"
#include "Engine.h"
#include "KeyConfig.h"
#include "Splitter.h"
#include "SyllableParser.h"
#include "TaiText.h"

namespace khiin::engine {
namespace {

std::string_view make_string_view(std::string const &str, std::string::const_iterator first,
                                  std::string::const_iterator last) noexcept {
    return std::string_view(str.data() + (first - str.begin()), last - first);
}

class SegmenterImpl : public Segmenter {
  public:
    SegmenterImpl(Engine *engine) : engine(engine) {}

    // Inherited via Segmenter
    virtual void GetBufferElements(std::string const &raw_buffer, std::vector<BufferElement> &result) override {
        m_result = &result;

        if (raw_buffer.empty() || m_result == nullptr) {
            return;
        }

        dictionary = engine->dictionary();
        splitter = engine->word_splitter();
        parser = engine->syllable_parser();

        auto it = raw_buffer.cbegin();
        auto end = raw_buffer.cend();

        bool khin_next = false;
        char hyphen_key = 0;
        auto khin_indices = std::vector<std::pair<size_t, char>>();

        while (it != end) {
            auto remaining_buffer = std::string(it, end);

            if (auto consumed = ConsumeHyphens(remaining_buffer, khin_next, hyphen_key); consumed > 0) {
                if (khin_next) {
                    khin_indices.push_back(std::make_pair(m_result->size(), hyphen_key));
                }
                khin_next = false;
                it += consumed;
                continue;
            }

            if (splitter->CanSplit(remaining_buffer)) {
                FlushPlaintext();
                ConsumeSplittableBuffer(remaining_buffer);
                break;
            }

            if (dictionary->IsWordPrefix(remaining_buffer)) {
                FlushPlaintext();
                ConsumeWordPrefix(remaining_buffer);
                break;
            }

            if (dictionary->IsSyllablePrefix(remaining_buffer)) {
                FlushPlaintext();
                ConsumeSyllable(remaining_buffer);
                break;
            }

            if (auto idx = CanSplitWithTrailingPrefix(remaining_buffer); idx > 0) {
                FlushPlaintext();
                ConsumeSplittableBuffer(remaining_buffer.substr(0, idx));
                ConsumeWordPrefix(remaining_buffer.substr(idx, remaining_buffer.size()));
                break;
            }

            if (dictionary->StartsWithWord(remaining_buffer) || dictionary->StartsWithSyllable(remaining_buffer)) {
                FlushPlaintext();
                if (auto consumed = ConsumeWordsOrSyllable(remaining_buffer); consumed > 0) {
                    it += consumed;
                    continue;
                }
            }

            pending_plaintext.push_back(*it);
            ++it;
        }

        FlushPlaintext();

        if (m_result->empty()) {
            return;
        }

        for (auto &index : khin_indices) {
            m_result->at(index.first).SetKhin(parser, KhinKeyPosition::Start, index.second);
        }

        for (auto i = result.size() - 1; i != 0; --i) {
            if (m_result->at(i).IsSpacerElement() || m_result->at(i - 1).IsSpacerElement()) {
                continue;
            }
            result.insert(result.begin() + i, BufferElement(Spacer::VirtualSpace));
        }
    }

    virtual void LongestFromStart(std::string_view raw_buffer, std::vector<BufferElement> &result) override {}

  private:
    void ConsumeSplittableBuffer(std::string input) {
        auto words = std::vector<std::string>();
        splitter->Split(input, words);
        for (auto &word : words) {
            auto best_match = dictionary->BestWord(word);
            if (best_match) {
                auto segment = parser->AsTaiText(word, best_match->input);
                segment.SetCandidate(best_match);
                m_result->push_back(BufferElement(segment));
            } else {
                ConsumeWordPrefix(word);
            }
        }
    }

    void ConsumeWordPrefix(std::string const &raw) {
        auto best_match = dictionary->BestAutocomplete(raw);
        if (best_match) {
            auto segment = parser->AsTaiText(raw, best_match->input);
            m_result->push_back(BufferElement(segment));
        } else {
            ConsumeAsRawSyllables(raw);
        }
    }

    void ConsumeAsRawSyllables(std::string const &raw) {
        auto start = raw.cbegin();
        auto it = start;
        auto end = raw.cend();
        while (it != end) {
            auto max = LongestSyllableIndex(std::string(start, end));
            if (max == 0) {
                pending_plaintext += std::string(start, end);
                break;
            }
            it += max;
            auto syl = std::string(start, it);
            ConsumeSyllable(syl);
            start = it;
        }
    }

    void ConsumeSyllable(std::string const &raw) {
        m_result->push_back(BufferElement(TaiText::FromRawSyllable(parser, raw)));
    };

    size_t ConsumeWordsOrSyllable(std::string const &input) {
        auto max_word = MaxSplittableWordIndex(input);
        auto max_syl = LongestSyllableIndex(input);

        if (max_syl > max_word) {
            auto syl = std::string(input.cbegin(), input.cbegin() + max_syl);
            ConsumeSyllable(syl);
            return max_syl;
        } else if (max_word != std::string::npos) {
            ConsumeSplittableBuffer(input.substr(0, max_word));
            return max_word;
        }

        return 0;
    }

    size_t ConsumeHyphens(std::string const &input, bool &khin_next, char &hyphen_key) {
        auto hyphen_keys = engine->key_configuration()->GetHyphenKeys();
        hyphen_key = 0;
        for (auto key : hyphen_keys) {
            if (input[0] == key) {
                hyphen_key = key;
                break;
            }
        }

        if (hyphen_key == 0) {
            return 0;
        }

        size_t n_hyphens = 0;
        for (auto c : input) {
            if (c == hyphen_key) {
                ++n_hyphens;
            }
        }

        FlushPlaintext();
        if (n_hyphens == 1 || n_hyphens == 3) {
            m_result->push_back(BufferElement(Spacer::Hyphen));
        }

        if (n_hyphens == 2 || n_hyphens == 3) {
            if (input.size() == n_hyphens) {
                Syllable syl;
                parser->ParseRaw(std::string(2, hyphen_key), syl);
                TaiText segment;
                segment.AddItem(syl);
                m_result->push_back(BufferElement(segment));
            } else {
                khin_next = true;
            }
        }

        if (n_hyphens > 3) {
            m_result->push_back(BufferElement(std::string(n_hyphens, '-')));
        }

        return n_hyphens;
    }

    void FlushPlaintext() {
        if (!pending_plaintext.empty()) {
            m_result->push_back(BufferElement(pending_plaintext));
            pending_plaintext.clear();
        }
    }

    size_t MaxSplittableWordIndex(std::string const &str) {
        for (auto i = str.size(); i > 0; --i) {
            if (splitter->CanSplit(str.substr(0, i))) {
                return i;
            }
        }

        return std::string::npos;
    }

    size_t LongestSyllableIndex(std::string_view query) {
        auto i = 1;
        auto size = query.size();
        while (i < size + 1) {
            if (dictionary->IsSyllablePrefix(query.substr(0, i))) {
                ++i;
                continue;
            }
            break;
        }
        return --i;
    }

    size_t CanSplitWithTrailingPrefix(std::string const& buffer) {
        auto start = buffer.cbegin();
        auto it = buffer.cend();
        auto end = buffer.cend();
        
        for (auto it = buffer.cend(); it != start; --it) {
            auto lhs = std::string(start, it);
            auto rhs = std::string(it, end);
            if (splitter->CanSplit(lhs) && dictionary->IsWordPrefix(rhs)) {
                return lhs.size();
            }
        }

        return 0;
    }

    Engine *engine = nullptr;
    Dictionary *dictionary = nullptr;
    Splitter *splitter = nullptr;
    SyllableParser *parser = nullptr;
    std::vector<BufferElement> *m_result = nullptr;
    std::string raw_buffer;
    std::string pending_plaintext;
};

} // namespace

Segmenter *Segmenter::Create(Engine *engine) {
    return new SegmenterImpl(engine);
}

} // namespace khiin::engine
