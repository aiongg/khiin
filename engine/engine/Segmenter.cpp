#include "Segmenter.h"

#include "Engine.h"
#include "CandidateFinder.h"

namespace khiin::engine {
namespace {

class SegmenterImpl : public Segmenter {
  public:
    SegmenterImpl(Engine *engine) : engine(engine) {}

    // Inherited via Segmenter
    virtual void SegmentWholeBuffer(std::string const &raw_buffer, size_t raw_caret, std::vector<BufferElement> &result,
                                    utf8_size_t &caret) override {
        // engine->candidate_finder()->...
        // engine->splitter()->...
        // engine->word_trie()->...
        // engine->syllable_trie()->...
    }

    virtual void LongestFromStart(std::string_view raw_buffer, std::vector<BufferElement> &result) override {}

  private:
    Engine *engine = nullptr;
};

} // namespace

Segmenter *Segmenter::Create(Engine *engine) {
    return new SegmenterImpl(engine);
}

} // namespace khiin::engine
