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

        auto splitter = engine->word_splitter();
        auto start = raw_buffer.cbegin();
        auto it = raw_buffer.cbegin();
        auto end = raw_buffer.cend();
        auto unknown_text = std::string();

        while (it != end) {
            auto remaining_buffer = std::string(it, end);

            if (engine->dictionary()->StartsWithWord(remaining_buffer)) {
                if (unknown_text.size()) {
                    result.push_back(BufferElement(unknown_text));
                    unknown_text.clear();
                }

                auto max = GetMaxSplittableIndex(remaining_buffer);
                if (max != std::string::npos) {
                    ConsumeSplittableBuffer(remaining_buffer.substr(0, max), result);
                    it += max;
                    continue;
                }
            }

            unknown_text.push_back(*it);
            ++it;
        }
    }

    virtual void LongestFromStart(std::string_view raw_buffer, std::vector<BufferElement> &result) override {}

  private:
    void ConsumeSplittableBuffer(std::string input, std::vector<BufferElement> &result) {
        auto words = std::vector<std::string>();
        engine->word_splitter()->Split(input, words);
        for (auto &word : words) {
            auto best_match = engine->dictionary()->BestWord(word);
            auto segment = engine->syllable_parser()->AsBufferSegment(word, best_match->input);
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

    Engine *engine = nullptr;
};

} // namespace

Segmenter *Segmenter::Create(Engine *engine) {
    return new SegmenterImpl(engine);
}

} // namespace khiin::engine
