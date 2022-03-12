#include "Segmenter.h"

#include "Dictionary.h"
#include "Engine.h"
#include "KeyConfig.h"
#include "Splitter.h"

namespace khiin::engine {
namespace {

size_t CheckHyphens(Engine *engine, std::string_view str) {
    auto hyphen_keys = engine->keyconfig()->GetHyphenKeys();

    size_t n_hyphens = 0;
    auto is_hyphen = false;
    for (auto c : str) {
        for (auto key : hyphen_keys) {
            if (c == key) {
                ++n_hyphens;
                is_hyphen = true;
                break;
            } else {
                is_hyphen = false;
            }
        }
        if (!is_hyphen) {
            break;
        }
    }

    return n_hyphens;
}

bool CheckSplittable(Engine *engine, std::string_view str) {
    return engine->dictionary()->word_splitter()->CanSplit(str);
}

bool CheckWordPrefix(Engine *engine, std::string_view str) {
    return engine->dictionary()->IsWordPrefix(str);
}

bool CheckSyllablePrefix(Engine *engine, std::string_view str) {
    return engine->dictionary()->IsSyllablePrefix(str);
}

size_t CheckSplittableWithTrailingPrefix(Engine *engine, std::string_view str) {
    auto dictionary = engine->dictionary();
    auto splitter = dictionary->word_splitter();
    auto start = str.begin();
    auto it = str.end();
    auto end = str.end();

    for (auto it = str.end(); it != start; --it) {
        auto lhs = std::string(start, it);
        auto rhs = std::string(it, end);
        if (splitter->CanSplit(lhs) && dictionary->IsWordPrefix(rhs)) {
            return lhs.size();
        }
    }

    return 0;
}

size_t MaxSyllable(Engine *engine, std::string_view str) {
    auto dictionary = engine->dictionary();

    if (!dictionary->StartsWithSyllable(str)) {
        return 0;
    }

    auto i = 1;
    auto size = str.size();
    while (i < size + 1) {
        if (dictionary->IsSyllablePrefix(str.substr(0, i))) {
            ++i;
            continue;
        }
        break;
    }
    return --i;
}

size_t MaxSplitSize(Engine *engine, std::string_view str) {
    return engine->dictionary()->word_splitter()->MaxSplitSize(str);
}

std::pair<size_t, SegmentType> CheckSyllableOrSplittable(Engine *engine, std::string_view str) {
    auto ret = std::make_pair(size_t(0), SegmentType::None);
    auto max_syl = MaxSyllable(engine, str);
    auto max_split = MaxSplitSize(engine, str);

    if (max_syl > 0 || max_split > 0) {
        if (max_syl > max_split) {
            ret.first = max_syl;
            ret.second = SegmentType::SyllablePrefix;
        } else {
            ret.first = max_split;
            ret.second = SegmentType::Splittable;
        }
    }

    return ret;
}

size_t CheckAsciiPunctuation(std::string_view str) {
    return unicode::start_glyph_type(str) == unicode::GlyphCategory::AsciiPunct ? 1 : 0;
}

std::vector<SegmentOffset> SegmentTextImpl(Engine *engine, std::string_view raw_buffer) {
    auto ret = std::vector<SegmentOffset>();
    auto begin = raw_buffer.begin();
    auto it = raw_buffer.begin();
    auto end = raw_buffer.end();

    size_t plaintext_start = 0;
    size_t plaintext_size = 0;

    auto flush_plaintext = [&]() {
        if (plaintext_size != 0) {
            ret.push_back(SegmentOffset{SegmentType::None, plaintext_start, plaintext_size});
            plaintext_start = 0;
            plaintext_size = 0;
        }
    };

    while (it != end) {
        auto index = static_cast<size_t>(std::distance(begin, it));
        auto remainder = raw_buffer.substr(index);

        if (auto size = CheckHyphens(engine, remainder); size > 0) {
            flush_plaintext();
            ret.push_back(SegmentOffset{SegmentType::Hyphens, index, size});
            it += size;
            continue;
        }

        if (auto size = CheckAsciiPunctuation(remainder); size > 0) {
            flush_plaintext();
            ret.push_back(SegmentOffset{SegmentType::Punct, index, size});
            it += size;
            continue;
        }

        if (CheckSplittable(engine, remainder)) {
            flush_plaintext();
            ret.push_back(SegmentOffset{SegmentType::Splittable, index, remainder.size()});
            break;
        }

        if (CheckWordPrefix(engine, remainder)) {
            flush_plaintext();
            ret.push_back(SegmentOffset{SegmentType::WordPrefix, index, remainder.size()});
            break;
        }

        if (CheckSyllablePrefix(engine, remainder)) {
            flush_plaintext();
            ret.push_back(SegmentOffset{SegmentType::SyllablePrefix, index, remainder.size()});
            break;
        }

        if (auto splits_at = CheckSplittableWithTrailingPrefix(engine, remainder); splits_at > 0) {
            flush_plaintext();
            ret.push_back(SegmentOffset{SegmentType::Splittable, index, splits_at});
            ret.push_back(SegmentOffset{SegmentType::WordPrefix, index + splits_at, remainder.size() - splits_at});
            break;
        }

        if (auto [size, type] = CheckSyllableOrSplittable(engine, remainder); size > 0) {
            flush_plaintext();
            ret.push_back(SegmentOffset{type, index, size});
            it += size;
            continue;
        }

        if (plaintext_size == 0) {
            plaintext_start = index;
        }

        auto tmp = it;
        utf8::unchecked::advance(tmp, 1);
        plaintext_size += std::distance(it, tmp);
        it = tmp;
    }

    flush_plaintext();
    return ret;
}

} // namespace

std::vector<SegmentOffset> Segmenter::SegmentText(Engine *engine, std::string_view raw_buffer) {
    auto lc = unicode::copy_str_tolower(raw_buffer);
    return SegmentTextImpl(engine, lc);
}

} // namespace khiin::engine
