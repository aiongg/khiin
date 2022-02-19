#pragma once

#include <variant>

#include "BufferElement.h"

namespace khiin::engine {

class Engine;

enum class BufferSegmentType {
    Splittable,
    WordPrefix,
    SyllablePrefix,
    SplittableWithTrailingPrefix,
    Hyphens,

};

// - Can find possible segmentations either for the entire buffer
//   or from the beginning of the buffer
// - Segmentation may be based on either dictionary Candidates or
//   user input (e.g. by typing a break character such as a tone, space, etc.)
class Segmenter {
  public:
    //static Segmenter *Create(Engine *engine);
    static void SegmentText(Engine *engine, std::string const &raw_buffer, std::vector<BufferElement> &result);

    //virtual void GetBufferElements(std::string const &raw_buffer, std::vector<BufferElement> &result) = 0;
    //virtual void LongestFromStart(std::string_view raw_buffer, std::vector<BufferElement> &result) = 0;
};

} // namespace khiin::engine
