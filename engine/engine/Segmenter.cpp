#include "Segmenter.h"

#include "CandidateFinder.h"
#include "Dictionary.h"
#include "Engine.h"
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
    virtual void SegmentWholeBuffer(std::string const &raw_buffer, size_t raw_caret, std::vector<BufferElement> &result,
                                    utf8_size_t &caret) override {
        if (raw_buffer.empty()) {
            return;
        }

        auto dictionary = engine->dictionary();
        auto splitter = engine->word_splitter();
        auto start = raw_buffer.cbegin();
        auto it = raw_buffer.cbegin();
        auto end = raw_buffer.cend();
        auto unknown_text = std::string();

        auto consume_unknown = [&]() {
            if (!unknown_text.empty()) {
                result.push_back(BufferElement(unknown_text));
                unknown_text.clear();
            }
        };

        auto consume_syllable = [&](const std::string &raw) {
            Syllable syllable;
            engine->syllable_parser()->ParseRaw(raw, syllable);
            TaiText segment;
            segment.AddItem(syllable);
            result.push_back(BufferElement(segment));
        };

        while (it != end) {
            auto remaining_buffer = std::string(it, end);

            if (splitter->CanSplit(remaining_buffer)) {
                consume_unknown();
                ConsumeSplittableBuffer(remaining_buffer, result);
                break;
            }

            if (dictionary->IsSyllablePrefix(remaining_buffer)) {
                consume_unknown();
                consume_syllable(remaining_buffer);
                break;
            }

            if (dictionary->StartsWithWord(remaining_buffer) || dictionary->StartsWithSyllable(remaining_buffer)) {
                consume_unknown();

                auto max_word = GetMaxSplittableIndex(remaining_buffer);
                auto max_syl = LongestSyllableFromStart(remaining_buffer);

                if (max_syl > max_word) {
                    consume_syllable(std::string(remaining_buffer.cbegin(), remaining_buffer.cbegin() + max_syl));
                    it += max_syl;
                    continue;
                } else if (max_word != std::string::npos) {
                    ConsumeSplittableBuffer(remaining_buffer.substr(0, max_word), result);
                    it += max_word;
                    continue;
                }
            }

            unknown_text.push_back(*it);
            ++it;
        }

        consume_unknown();

        if (result.empty()) {
            return;
        }

        for (auto i = result.size() - 1; i != 0; --i) {
            result.insert(result.begin() + i, BufferElement(Spacer::VirtualSpace));
        }
    }

    virtual void LongestFromStart(std::string_view raw_buffer, std::vector<BufferElement> &result) override {}

  private:
    void ConsumeSplittableBuffer(std::string input, std::vector<BufferElement> &result) {
        auto words = std::vector<std::string>();
        engine->word_splitter()->Split(input, words);
        for (auto &word : words) {
            auto best_match = engine->dictionary()->BestWord(word);
            auto segment = engine->syllable_parser()->AsTaiText(word, best_match->input);
            segment.SetCandidate(best_match);
            result.push_back(BufferElement(segment));
        }
    }

    size_t GetMaxSplittableIndex(std::string const &str) {
        auto splitter = engine->word_splitter();

        for (auto i = str.size(); i > 0; --i) {
            if (splitter->CanSplit(str.substr(0, i))) {
                return i;
            }
        }

        return std::string::npos;
    }

    size_t LongestSyllableFromStart(std::string_view query) {
        auto i = 1;
        auto size = query.size();
        while (i != size) {
            if (engine->dictionary()->IsSyllablePrefix(query.substr(0, i))) {
                ++i;
                continue;
            }
            break;
        }
        return --i;
    }

    Engine *engine = nullptr;
};

} // namespace

Segmenter *Segmenter::Create(Engine *engine) {
    return new SegmenterImpl(engine);
}

} // namespace khiin::engine
