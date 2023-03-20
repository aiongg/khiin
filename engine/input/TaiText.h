#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "data/Models.h"
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
    static TaiText FromMatching(SyllableParser *parser, std::string const &input, TaiToken const &match);

    bool operator==(TaiText const &rhs) const;

    void AddItem(Syllable syllable);
    void AddItem(VirtualSpace spacer);
    void SetCandidate(TaiToken const &candidate);

    std::string RawText() const;
    utf8_size_t RawSize() const;
    std::string ComposedText() const;
    utf8_size_t ComposedSize() const;
    std::string ConvertedText() const;
    utf8_size_t ConvertedSize() const;
    size_t SyllableSize() const;

    utf8_size_t RawToComposedCaret(size_t raw_caret) const;
    size_t ComposedToRawCaret(utf8_size_t caret) const;
    size_t ConvertedToRawCaret(utf8_size_t caret) const;

    void Erase(utf8_size_t index);
    bool IsVirtualSpace(utf8_size_t index) const;
    void SetKhin(KhinKeyPosition khin_pos, char khin_key);

    std::optional<TaiToken> candidate;

  private:
    std::vector<Chunk> m_elements;
};

} // namespace khiin::engine
