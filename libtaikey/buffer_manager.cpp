#include <regex>
#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <c9/zip.h>
#include <utf8cpp/utf8.h>

#include "buffer_manager.h"
#include "common.h"
#include "lomaji.h"
#include "trie.h"

namespace TaiKey {

// Local utility methods

std::regex toneableLetters(u8"[aeioumn]");

auto isOnlyHyphens(std::string s) {
    for (const auto &c : s) {
        if (c != '-') {
            return false;
        }
    }

    return true;
}

auto candidateDisplayOf(Candidates candidate) -> CandidateDisplay {
    auto ret = CandidateDisplay();

    auto tokens = std::vector<std::string_view>();
    std::transform(candidate.cbegin(), candidate.cend(),
                   std::back_inserter(tokens), [](const Candidate &token) {
                       if (token.output.empty()) {
                           return std::string_view(token.ascii);
                       }
                       return std::string_view(token.output);
                   });

    auto spaces = tokenSpacer(tokens);

    for (auto &&[spaced, token] : c9::zip(spaces, tokens)) {
        ret.text.append(token);
        if (spaced) {
            ret.text.push_back(' ');
        }
    }

    return ret;
}

// BufferManager

BufferManager::BufferManager(CandidateFinder *candidateFinder)
    : candidateFinder(candidateFinder) {}

// Public

auto BufferManager::clear() -> RetVal {
    buffer.clear();

    return RetVal::TODO;
}

auto BufferManager::focusCandidate(size_t index) -> RetVal {
    if (index >= candidates.size()) {
        throw std::invalid_argument("Candidate index out of range");
    }

    return RetVal::TODO;
}

auto BufferManager::getDisplayBuffer() -> std::string {
    return buffer.displayText();
}

auto BufferManager::getCursor() -> size_t {
    return buffer.displayCursorOffset();
}

auto BufferManager::getCandidates() -> std::vector<CandidateDisplay> {
    auto ret = std::vector<CandidateDisplay>();

    for (auto &c : candidates) {
        ret.emplace_back(candidateDisplayOf(c));
    }

    return ret;
}

auto BufferManager::insert(char ch) -> RetVal {
    /*
     * ch must match: [A-Za-z0-9-]
     *
     * Cases:
     *   1. Normal / Pro input mode
     *   2. Numeric / Telex tones
     *   3. Fuzzy / Exact tones
     *   4. Lazy / Quick commit
     */
    switch (inputMode_) {
    case InputMode::Normal:
        return insertNormal(ch);
    }

    return RetVal::NotConsumed;
}

auto BufferManager::moveCursor(CursorDirection dir) -> RetVal {
    buffer.moveCursor(dir);
    return RetVal::Consumed;
}

auto BufferManager::erase(CursorDirection dir) -> RetVal {
    buffer.erase(dir);
    findPrimaryCandidate(buffer.segmentAtCursor(), buffer.segmentEnd());
    return RetVal::TODO;
}

auto BufferManager::setToneKeys(ToneKeys toneKeys) -> RetVal {
    toneKeys_ = toneKeys;
    return RetVal::TODO;
}

auto BufferManager::selectPrimaryCandidate() -> RetVal {
    if (!hasPrimaryCandidate) {
        return RetVal::TODO;
    }

    auto seg_it = buffer.editingBegin();
    auto cand_it = candidates[0].begin();

    for (auto i = 0; i < candidates[0].size(); ++i) {
        auto &segment = *seg_it;
        auto &cand = *cand_it;

        if (!cand.output.empty()) {
            segment.display = cand.output;
        }

        segment.selected = true;

        ++seg_it;
        ++cand_it;
    }

    buffer.updateSegmentSpacing();
    buffer.moveCursorToEnd();

    return RetVal::TODO;
}

auto BufferManager::selectCandidate(size_t index) -> RetVal {
    if (index == 0 && hasPrimaryCandidate) {
        return selectPrimaryCandidate();
    }

    auto &selected = candidates[index];
    return RetVal::TODO;
}

// Private
auto BufferManager::empty() -> bool { return false; /* TODO */ }

auto BufferManager::isCursorAtEnd() -> bool { return buffer.isCursorAtEnd(); }

auto BufferManager::findCandidates(SegmentIter begin) -> void {
    auto lgram = lgramOf(begin);
    auto searchStr = buffer.rawText(begin, buffer.editingEnd());
    auto fuzzy = toneMode == ToneMode::Fuzzy;

    auto found = candidateFinder->findCandidates(searchStr, fuzzy, lgram);

    if (found.empty()) {
        return;
    }

    if (hasPrimaryCandidate) {
        auto &pc = candidates[0];
        found.erase(std::remove_if(found.begin(), found.end(),
                                   [&](const Candidate cand) {
                                       return cand == pc[0];
                                   }),
                    found.end());
    }

    for (auto &alt : found) {
        candidates.emplace_back(Candidates{std::move(alt)});
    }
}

auto BufferManager::findPrimaryCandidate(SegmentIter begin, SegmentIter last)
    -> void {
    auto lgram = lgramOf(begin);
    auto searchStr = buffer.rawText(begin, last);
    auto fuzzy = toneMode == ToneMode::Fuzzy;

    auto primary =
        candidateFinder->findPrimaryCandidate(searchStr, fuzzy, lgram);

    if (primary.empty()) {
        return;
    }

    buffer.segmentByCandidateList(begin, last, primary);
    // buffer.setCandidates(first, std::move(candidates));

    candidates.emplace_back(std::move(primary));
    hasPrimaryCandidate = true;
}

auto BufferManager::insertNormal(char ch) -> RetVal {
    SegmentIter begin = buffer.segmentBegin();
    SegmentIter curs = buffer.segmentAtCursor();

    if (buffer.isCursorAtEnd()) {
        if (buffer.segmentCount() > 4) {
            begin = curs - 4;
        } else {
            begin = buffer.editingBegin();
        }
    } else {
        begin = curs;
    }

    buffer.insert(ch);
    candidates.clear();
    findPrimaryCandidate(begin, buffer.editingEnd());
    findCandidates(buffer.editingBegin());

    return RetVal::TODO;
}

auto BufferManager::insertTelex_(char ch) -> RetVal { return RetVal::Error; }

auto BufferManager::lgramOf(SegmentIter segment) -> std::string {
    return segment == buffer.segmentBegin() ? std::string() : segment[-1].display;
}

} // namespace TaiKey
