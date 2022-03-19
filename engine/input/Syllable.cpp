#include "Syllable.h"

#include <assert.h>

#include <utf8cpp/utf8.h>

#include "config/KeyConfig.h"
#include "utils/unicode.h"

namespace khiin::engine {
namespace {
namespace u8u = utf8::unchecked;
using namespace khiin::unicode;

const std::string kKhinDotStr = u8"\u00b7";
const std::string kKhinHyphenStr = "--";

} // namespace

Syllable::Syllable(Syllable const &other) = default;

Syllable::Syllable(KeyConfig *keyconfig, bool dotted_khin) : m_keyconfig(keyconfig), m_dotted_khin(dotted_khin) {}

void Syllable::SetRawInput(std::string const &input) {
    m_raw_input = input;
    m_raw_body = input;
    ExtractRawKhin();
    ExtractRawTone();
    BuildComposed();
}

void Syllable::SetComposed(std::string const &input) {
    m_composed = input;
    BuildRaw();
}

utf8_size_t Syllable::RawToComposedCaret(size_t raw_caret) const {
    if (raw_caret == 0) {
        return 0;
    }

    auto ret = std::string::npos;

    auto raw_size = u8_size(m_raw_input);
    auto composed_size = u8_size(m_composed);

    if (raw_caret == raw_size) {
        ret = u8_size(m_composed);
    } else if (raw_caret < raw_size) {
        auto lhs = std::string(m_raw_input.cbegin(), m_raw_input.cbegin() + raw_caret);

        if (m_tone != Tone::NaT) {
            size_t tone_pos = Lomaji::FindTonePosition(m_raw_input);
            if (tone_pos <= lhs.size()) {
                lhs.insert(lhs.end(), m_tone_key);
            }
        }

        auto lhs_syl = Syllable(*this);
        lhs_syl.SetRawInput(lhs);
        ret = u8_size(lhs_syl.m_composed);
    }

    return ret;
}

size_t Syllable::ComposedToRawCaret(utf8_size_t composed_caret) const {
    size_t size = utf8::distance(m_composed.cbegin(), m_composed.cend());
    auto ret = std::string::npos;

    if (composed_caret == size) {
        ret = u8_size(m_raw_input);
    } else if (composed_caret < size) {
        auto caret = m_composed.cbegin();
        u8u::advance(caret, composed_caret);
        auto lhs = std::string(m_composed.cbegin(), caret);
        Lomaji::RemoveToneDiacritic(lhs);
        m_keyconfig->Deconvert(lhs);

        if (m_khin_pos == KhinKeyPosition::Virtual || m_khin_pos == KhinKeyPosition::End) {
            Lomaji::RemoveKhin(lhs);
        } else if (m_khin_pos == KhinKeyPosition::Start) {
            Lomaji::ReplaceKhinDot(lhs);
        }

        ret = u8_size(lhs);
    }

    return ret;
}

void Syllable::Clear() {
    m_composed.clear();
    m_raw_input.clear();
    m_raw_body.clear();
    m_tone = Tone::NaT;
    m_tone_key = 0;
    m_khin_pos = KhinKeyPosition::None;
    m_khin_key = 0;
}

bool Syllable::Empty() const {
    return m_composed.empty() && m_raw_input.empty() && m_raw_body.empty() && m_tone == Tone::NaT && m_tone_key == 0 &&
           m_khin_pos == KhinKeyPosition::None && m_khin_key == 0;
}

void Syllable::Erase(utf8_size_t index) {
    auto size = u8_size(m_composed);
    if (index > size) {
        return;
    }

    auto from = m_composed.begin();
    u8u::advance(from, index);

    auto curs_end = Lomaji::MoveCaret(m_composed, index, CursorDirection::R);
    auto to = m_composed.begin();
    u8u::advance(to, curs_end);

    if (Lomaji::HasToneDiacritic(std::string(from, to))) {
        m_tone = Tone::NaT;
        m_tone_key = 0;
    }

    m_composed.erase(from, to);

    if (m_composed.empty()) {
        Clear();
        return;
    }

    BuildRaw();
}

void Syllable::SetKhin(KhinKeyPosition khin_pos, char khin_key) {
    auto &khinstr = m_dotted_khin ? kKhinDotStr : kKhinHyphenStr;
    if (m_khin_pos == KhinKeyPosition::None && khin_pos != KhinKeyPosition::None) {
        if (khin_pos != KhinKeyPosition::Virtual) {
            m_raw_input.insert(0, 2, khin_key);
        }
        m_composed.insert(0, khinstr);
    } else if (m_khin_pos != KhinKeyPosition::None && khin_pos == KhinKeyPosition::None &&
               m_composed.find(khinstr) == 0) {
        if (m_khin_pos == KhinKeyPosition::Start) {
            m_raw_input.erase(0, 2);
        } else if (m_khin_pos == KhinKeyPosition::End) {
            m_raw_input.pop_back();
        }
        m_composed.erase(0, khinstr.size());
    }

    m_khin_pos = khin_pos;
    m_khin_key = khin_key;
}

bool Syllable::operator==(Syllable const &rhs) const {
    return m_raw_body == rhs.m_raw_body && m_tone == rhs.m_tone && m_khin_pos == rhs.m_khin_pos;
}

std::string Syllable::raw_input() const {
    return m_raw_input;
}

Tone Syllable::tone() const {
    return m_tone;
}

std::string Syllable::composed() const {
    return m_composed;
}

size_t Syllable::composed_size() const {
    return u8_size(m_composed);
}

std::string Syllable::raw_body() const {
    return m_raw_body;
}

size_t Syllable::raw_body_size() const {
    return u8_size(m_raw_body);
}

char Syllable::tone_key() const {
    return m_tone_key;
}

char Syllable::khin_key() const {
    return m_khin_key;
}

KhinKeyPosition Syllable::khin_pos() const {
    return m_khin_pos;
}

size_t Syllable::raw_input_size() const {
    return u8_size(m_raw_input);
}

// private methods

void Syllable::ExtractRawKhin() {
    auto &str = m_raw_body;

    if (str.empty()) {
        return;
    }

    if (str.size() > 1) {
        auto hyphen_keys = m_keyconfig->GetHyphenKeys();
        for (auto key : hyphen_keys) {
            if (str[0] == key && str[1] == key) {
                str.erase(0, 2);
                m_khin_pos = KhinKeyPosition::Start;
                m_khin_key = key;
                return;
            }
        }

        auto khin_keys = m_keyconfig->GetKhinKeys();
        for (auto key : khin_keys) {
            if (str.back() == key) {
                str.pop_back();
                m_khin_pos = KhinKeyPosition::End;
                m_khin_key = key;
                return;
            }
        }
    }
}

void Syllable::ExtractRawTone() {
    auto &str = m_raw_body;
    if (!Lomaji::HasToneable(str)) {
        return;
    }

    m_tone = m_keyconfig->CheckToneKey(str.back());
    if (m_tone == Tone::NaT) {
        return;
    }

    m_tone_key = str.back();
    str.pop_back();
    return;
}

void Syllable::EnsureKhinKey() {
    if (m_khin_key == 0) {
        switch (m_khin_pos) {
        case KhinKeyPosition::None:
            m_khin_key = m_keyconfig->GetHyphenKeys()[0];
            m_khin_pos = KhinKeyPosition::Start;
            break;
        case KhinKeyPosition::Start:
            m_khin_key = m_keyconfig->GetHyphenKeys()[0];
            break;
        case KhinKeyPosition::End:
            m_khin_key = m_keyconfig->GetKhinKeys()[0];
            break;
        }
    }

    if (m_khin_pos == KhinKeyPosition::None) {
        auto hyphen_keys = m_keyconfig->GetHyphenKeys();
        if (auto it = std::find(hyphen_keys.begin(), hyphen_keys.end(), m_khin_key); it != hyphen_keys.end()) {
            m_khin_pos = KhinKeyPosition::Start;
        } else {
            auto khin_keys = m_keyconfig->GetKhinKeys();
            if (auto it = std::find(khin_keys.begin(), khin_keys.end(), m_khin_key); it != khin_keys.end()) {
                m_khin_pos = KhinKeyPosition::End;
            }
        }
    }
}

void Syllable::EnsureToneKey() {
    if (m_tone_key == 0) {
        char telex_key = 0;
        m_keyconfig->GetToneKeys(m_tone, m_tone_key, telex_key);
    }
}

void Syllable::BuildComposed() {
    auto composed = m_keyconfig->Convert(m_raw_body);
    Lomaji::ApplyToneDiacritic(m_tone, composed);
    if (m_khin_pos != KhinKeyPosition::None) {
        auto &khinstr = m_dotted_khin ? kKhinDotStr : kKhinHyphenStr;
        composed.insert(0, khinstr);
    }
    m_composed = to_nfc(composed);
}

void Syllable::BuildRaw() {
    if (m_composed == "-") {
        Clear();
        m_composed = "-";
        m_raw_input = "-";
        return;
    }

    m_raw_body = m_composed;

    if (m_tone = Lomaji::RemoveToneDiacritic(m_raw_body); m_tone != Tone::NaT) {
        EnsureToneKey();
    }

    if (Lomaji::RemoveKhin(m_raw_body)) {
        EnsureKhinKey();
    }

    m_raw_body = m_keyconfig->Deconvert(m_raw_body);

    m_raw_input = m_raw_body;

    if (m_tone != Tone::NaT) {
        m_raw_input.push_back(m_tone_key);
    }

    if (m_khin_pos == KhinKeyPosition::Start) {
        m_raw_input.insert(0, 2, m_khin_key);
    }

    if (m_khin_pos == KhinKeyPosition::End) {
        m_raw_input.push_back(m_khin_key);
    }
}

} // namespace khiin::engine
