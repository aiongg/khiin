#pragma once

#include <variant>
#include <vector>

#include "Models.h"
#include "Syllable.h"

namespace khiin::engine {
class SyllableParser;

struct VirtualSpace {
    bool erased = false;
};

// A series of Tai Text syllables and spacers that makes up a single
// segment on the buffer (e.g., has a single candidate)
class TaiText {
  public:
    using Chunk = std::variant<Syllable, VirtualSpace>;
    static TaiText FromRawSyllable(SyllableParser *parser, std::string const &syllable);
    static TaiText FromMatching(SyllableParser *parser, std::string const &input, TaiToken *match);

    void AddItem(Syllable syllable);
    void AddItem(VirtualSpace spacer);
    void SetCandidate(TaiToken *candidate);

    utf8_size_t size() const;
    std::string raw() const;
    std::string composed() const;
    std::string converted() const;
    TaiToken *candidate() const;

    utf8_size_t RawToComposedCaret(SyllableParser *parser, size_t raw_caret) const;
    size_t ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret) const;
    size_t ConvertedToRawCaret(SyllableParser *parser, utf8_size_t caret) const;

    void Erase(SyllableParser *parser, utf8_size_t index);
    bool IsVirtualSpace(utf8_size_t index) const;
    void SetKhin(SyllableParser *parser, KhinKeyPosition khin_pos, char khin_key);

  private:
    std::vector<Chunk> m_elements;
    TaiToken *m_candidate = nullptr;
};

} // namespace khiin::engine
