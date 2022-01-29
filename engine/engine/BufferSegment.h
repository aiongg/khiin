#pragma once

#include <variant>
#include <vector>

#include "Syllable.h"

namespace khiin::engine {

enum class Spacer {
    Hyphen,
    Space,
    Zwd,
};

class BufferSegment {
  private:

  public:
    using SegmentElement = std::variant<Syllable, Spacer>;
    utf8_size_t Size();

    // RAW:  peng5an
    // COMP: pêng an
    // DICT: pêng-an

    //void Create(std::string raw, std::string dictionary);
    // --> Syllable(peng), Spacer(Hyphen), Syllable(an)

  private:
    std::vector<SegmentElement> m_elements;
};

} // namespace khiin::engine
