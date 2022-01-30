#include "Segmenter.h"

#include "CandidateFinder.h"
#include "Dictionary.h"
#include "Engine.h"

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
        auto it = raw_buffer.begin();
        auto caret_it = it + raw_caret;
        auto end = raw_buffer.end();

        while (it != end) {
            auto view = make_string_view(raw_buffer, it, end);
            auto id = 0;
            size_t consumed = 0;
            engine->candidate_finder()->FindBestCandidate(view, "", id, consumed);

        }

        // engine->candidate_finder()->...
        // engine->splitter()->...
        // engine->dictionary()->...
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
