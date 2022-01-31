#pragma once

#include <variant>
#include <vector>

#include "Models.h"
#include "Syllable.h"

namespace khiin::engine {

enum class Spacer {
    Hyphen,
    Space,
    Zwd,
};

// A series of Tai Text syllables and spacers that makes up a single
// segment on the buffer (e.g., has a single candidate)
class BufferSegment {
  public:
    using SegmentElement = std::variant<Syllable, Spacer>;
    void AddItem(Syllable syllable);
    void AddItem(Spacer spacer);
    utf8_size_t Size();
    std::string Raw();
    void RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret);

  private:
    std::vector<SegmentElement> m_elements;
    DictionaryRow *candidate = nullptr;
};

} // namespace khiin::engine
