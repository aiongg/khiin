#pragma once

#include <variant>
#include <vector>

#include "Models.h"
#include "Syllable.h"

namespace khiin::engine {
class SyllableParser;

enum class Spacer {
    None,
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
    static TaiText FromRawSyllable(SyllableParser *parser, std::string const &syllable);
    void AddItem(Syllable syllable);
    void AddItem(Spacer spacer);
    void SetCandidate(DictionaryRow *candidate);

    utf8_size_t size() const;
    std::string raw() const;
    std::string composed() const;
    std::string converted() const;

    void RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret) const;
    utf8_size_t RawToComposedCaret(SyllableParser *parser, size_t raw_caret) const;
    size_t ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret) const;
    void Erase(SyllableParser *parser, utf8_size_t index);
    bool IsVirtualSpace(utf8_size_t index) const;
    void SetKhin(SyllableParser *parser, KhinKeyPosition khin_pos, char khin_key);


  private:
    std::vector<Chunk> m_elements;
    DictionaryRow *candidate = nullptr;
};

} // namespace khiin::engine
