#pragma once

#include <string>

#include "Lomaji.h"

namespace khiin::engine {

enum class KhinKeyPosition {
    None,
    Start,
    End,
    Virtual,
};

class KeyConfig;

struct Syllable {
    Syllable(Syllable const &other);
    Syllable(KeyConfig *keyconfig, bool dotted_khin);
    bool operator==(Syllable const &rhs) const;
    void SetRawInput(std::string const &input);
    void SetComposed(std::string const &input);
    void SetKhin(KhinKeyPosition khin_pos, char khin_key);
    void Erase(utf8_size_t index);
    utf8_size_t RawToComposedCaret(size_t raw_caret) const;
    size_t ComposedToRawCaret(utf8_size_t composed_caret) const;
    void Clear();
    bool Empty() const;

    std::string raw_input() const;
    size_t raw_input_size() const;
    Tone tone() const;
    std::string composed() const;
    size_t composed_size() const;

    std::string raw_body() const;
    size_t raw_body_size() const;
    char tone_key() const;
    char khin_key() const;
    KhinKeyPosition khin_pos() const;

  private:
    void ExtractRawKhin();
    void ExtractRawTone();
    void EnsureKhinKey();
    void EnsureToneKey();
    void BuildComposed();
    void BuildRaw();

    KeyConfig *m_keyconfig;
    bool m_dotted_khin = false;
    std::string m_raw_input;
    std::string m_raw_body;
    Tone m_tone = Tone::NaT;
    KhinKeyPosition m_khin_pos = KhinKeyPosition::None;
    char m_tone_key = 0;
    char m_khin_key = 0;
    std::string m_composed;
};

} // namespace khiin::engine
