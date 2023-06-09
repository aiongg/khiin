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
    Punct,
    UserItem,
};

struct SegmentOffset {
    SegmentType type = SegmentType::None;
    size_t start = 0;
    size_t size = 0;
};

class Segmenter {
  public:
    static std::vector<SegmentOffset> SegmentText(Engine *engine, std::string_view raw_buffer);
    static SegmentOffset LongestSegmentFromStart(Engine *engine, std::string_view raw_buffer);
};

} // namespace khiin::engine
