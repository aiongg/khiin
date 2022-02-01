#include "Segmenter.h"

#include "BufferSegment.h"
#include "CandidateFinder.h"
#include "Dictionary.h"
#include "Engine.h"
#include "Splitter.h"
#include "SyllableParser.h"

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
        size_t max_splittable_idx = GetMaxSplittableIndex(raw_buffer);
        auto split = string_vector();
        engine->word_splitter()->Split(raw_buffer, split);
        for (auto &ea : split) {
            auto best_word = engine->dictionary()->BestWord(ea);
            auto segment = engine->syllable_parser()->AsBufferSegment(ea, best_word->input);
            auto elem = BufferElement(ea);
            result.push_back(elem);
        }
    }

    virtual void LongestFromStart(std::string_view raw_buffer, std::vector<BufferElement> &result) override {}

  private:
    size_t GetMaxSplittableIndex(std::string const &buffer) {
        auto splitter = engine->word_splitter();

        for (auto i = buffer.size(); i > 0; --i) {
            if (splitter->CanSplit(buffer.substr(0, i))) {
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
