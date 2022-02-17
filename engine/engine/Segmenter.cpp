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

class SegmenterImpl {
  public:
    void ProcessBuffer(Engine *engine, std::string const *raw_buffer, std::vector<BufferElement> *result) {
        m_engine = engine;
        m_buffer = raw_buffer;
        m_result = result;
        dictionary = engine->dictionary();
        splitter = dictionary->word_splitter();
        parser = engine->syllable_parser();
        keyconfig = engine->key_configuration();

        auto it = m_buffer->cbegin();
        auto end = m_buffer->cend();

        while (it != end) {
            auto remainder = std::string(it, end);
            auto consumed = TryConsume(remainder);

            if (consumed > 0) {
                it += consumed;
            } else {
                auto tmp = it;
                utf8::unchecked::next(tmp);
                pending_plaintext.append(std::string(it, tmp));
                it = tmp;
            }
        }

        FlushPlaintext();
    }

  private:
    size_t TryConsume(std::string const &remainder) {
        if (auto consumed = ConsumeHyphens(remainder); consumed > 0) {
            return consumed;
        }

        if (splitter->CanSplit(remainder)) {
            ConsumeSplittableBuffer(remainder);
            return remainder.size();
        }

        if (dictionary->IsWordPrefix(remainder)) {
            ConsumeWordPrefix(nullptr, remainder);
            return remainder.size();
        }

        if (dictionary->IsSyllablePrefix(remainder)) {
            ConsumeSyllable(remainder);
            return remainder.size();
        }

        if (auto idx = CanSplitWithTrailingPrefix(remainder); idx > 0) {
            ConsumeSplittableBuffer(remainder.substr(0, idx));
            ConsumeWordPrefix(nullptr, remainder.substr(idx, remainder.size()));
            return remainder.size();
        }

        if (dictionary->StartsWithWord(remainder) || dictionary->StartsWithSyllable(remainder)) {
            if (auto consumed = ConsumeWordsOrSyllable(remainder); consumed > 0) {
                return consumed;
            }
        }

        return 0;
    }

    void ConsumeSplittableBuffer(std::string input) {
        auto words = std::vector<std::string>();
        splitter->Split(input, words);
        TaiToken *prev_best = nullptr;
        for (auto &word : words) {
            auto best_match = CandidateFinder::BestMatch(m_engine, prev_best, word);
            if (best_match) {
                AddElement(BufferElement(TaiText::FromMatching(parser, word, best_match)));
            } else {
                ConsumeWordPrefix(prev_best, word);
            }
            prev_best = best_match;
        }
    }

    void ConsumeWordPrefix(TaiToken *lgram, std::string const &raw) {
        if (lgram == nullptr && m_result->size()) {
            lgram = m_result->back().candidate();
        }

        auto best_match = CandidateFinder::BestAutocomplete(m_engine, lgram, raw);
        if (best_match) {
            AddElement(BufferElement(TaiText::FromMatching(parser, raw, best_match)));
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
        AddElement(BufferElement(TaiText::FromRawSyllable(parser, raw)));
    };

    size_t ConsumeWordsOrSyllable(std::string const &input) {
        auto max_word = splitter->MaxSplitSize(input);
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

    size_t ConsumeHyphens(std::string const &input) {
        auto hyphen_keys = keyconfig->GetHyphenKeys();

        size_t n_hyphens = 0;
        auto is_hyphen = false;
        for (auto c : input) {
            for (auto key : hyphen_keys) {
                if (c == key) {
                    ++n_hyphens;
                    is_hyphen = true;
                    break;
                } else {
                    is_hyphen = false;
                }
            }
            if (!is_hyphen) {
                break;
            }
        }

        if (n_hyphens > 0) {
            AddElement(BufferElement(std::string(n_hyphens, '-')));
        }

        return n_hyphens;
    }

    void FlushPlaintext() {
        if (!pending_plaintext.empty()) {
            m_result->push_back(BufferElement(pending_plaintext));
            pending_plaintext.clear();
        }
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

    size_t CanSplitWithTrailingPrefix(std::string const &buffer) {
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

    void AddElement(BufferElement &&elem) {
        FlushPlaintext();
        m_result->push_back(elem);
    }

    Engine *m_engine = nullptr;
    std::vector<BufferElement> *m_result = nullptr;
    std::string const *m_buffer = nullptr;

    Dictionary *dictionary = nullptr;
    Splitter *splitter = nullptr;
    SyllableParser *parser = nullptr;
    KeyConfig *keyconfig = nullptr;

    std::string pending_plaintext;
};

} // namespace

void Segmenter::SegmentText(Engine *engine, std::string const &raw_buffer, std::vector<BufferElement> &result) {
    auto impl = SegmenterImpl();
    impl.ProcessBuffer(engine, &raw_buffer, &result);
}

} // namespace khiin::engine
