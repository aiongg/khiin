#pragma once

#include <variant>

#include "BufferElement.h"

namespace khiin::engine {

class Engine;

enum class SegmentType {
    None,
    Splittable,
    WordPrefix,
    SyllablePrefix,
    Hyphens,
};

struct SegmentOffset {
    SegmentType type = SegmentType::None;
    size_t start = 0;
    size_t size = 0;
};

class Segmenter {
  public:
    static std::vector<SegmentOffset> SegmentText2(Engine *engine, std::string_view raw_buffer);
};

} // namespace khiin::engine
