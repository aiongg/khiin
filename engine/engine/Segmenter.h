#pragma once

#include <variant>

#include "BufferElement.h"

namespace khiin::engine {

class Engine;

// - Can find possible segmentations either for the entire buffer
//   or from the beginning of the buffer
// - Segmentation may be based on either dictionary Candidates or
//   user input (e.g. by typing a break character such as a tone, space, etc.)
class Segmenter {
  public:
    //static Segmenter *Create(Engine *engine);
    static void SegmentWholeBuffer(Engine *engine, std::string const &raw_buffer, std::vector<BufferElement> &result);

    //virtual void GetBufferElements(std::string const &raw_buffer, std::vector<BufferElement> &result) = 0;
    //virtual void LongestFromStart(std::string_view raw_buffer, std::vector<BufferElement> &result) = 0;
};

} // namespace khiin::engine

// Misc notes from a call:
// pe nan -> peng an
// penan -> peng an

// RAW:       PENAN
// COMPOSING: PE NA N
// CARET:         ^
// INSERT:        G
// RAW:       PENGAN
// RAW INSERT:   ^           A
// COMPOSING: PENG AN
// CARET:         ^          B

// COMPOSING BUFFER + COMPOSING CARET POSITION
//               -------> RAW BUFFER + RAW CARET POSITION
//               -------> INSERT KEY AT RAW CARET POSITION
//               -------> CUT INTO COMPOSING BUFFER & COMPOSING CARET POSITION