#pragma once

#include <variant>
#include <vector>

#include "Models.h"
#include "Syllable.h"

namespace khiin::engine {
class SyllableParser;

enum class Spacer {
    Hyphen,
    Space,
    Zwd,
    VirtualSpace
};

// A series of Tai Text syllables and spacers that makes up a single
// segment on the buffer (e.g., has a single candidate)
class TaiText {
  public:
    using Chunk = std::variant<Syllable, Spacer>;
    void AddItem(Syllable syllable);
    void AddItem(Spacer spacer);
    void SetCandidate(DictionaryRow *candidate);

    utf8_size_t size();
    std::string raw();
    std::string composed();

    void RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret);
    utf8_size_t RawToComposedCaret(SyllableParser *parser, size_t raw_caret);
    size_t ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret);

  private:
    std::vector<Chunk> m_elements;
    DictionaryRow *candidate = nullptr;
};

} // namespace khiin::engine
