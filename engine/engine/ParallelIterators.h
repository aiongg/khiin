#pragma once

#include "Lomaji.h"

namespace khiin::engine {

namespace {

auto static codepointAsciiSize = std::unordered_map<uint32_t, int>{
    {' ', 0},    {U32_T2, 0}, {U32_T3, 0}, {U32_T5, 0},    {U32_T7, 0}, {U32_T8, 0},
    {U32_T9, 0}, {U32_TK, 2}, {U32_NN, 2}, {U32_NN_UC, 2}, {U32_UR, 2}, {U32_UR_UC, 2}};

auto static asciiLettersPerCodepoint(uint32_t cp) {
    if (codepointAsciiSize.find(cp) != codepointAsciiSize.end()) {
        return codepointAsciiSize.at(cp);
    }

    return 1;
}

auto static cursorSkipsCodepoint(uint32_t cp) {
    return 0x0300 <= cp && cp <= 0x0358;
}

} // namespace

template <typename iter_t>
void ParallelAdvance(iter_t &a_it, iter_t &a_end, iter_t &u_it, iter_t &u_end) {
    while (u_it != u_end && (utf8::peek_next(u_it, u_end) == '-' || utf8::peek_next(u_it, u_end) == ' ')) {
        utf8::advance(u_it, 1, u_end);

        if (a_it != a_end && a_it + 1 != a_end && *(a_it + 1) == '-') {
            a_it++;
        }

        return;
    }

    while (a_it != a_end && a_it + 1 != a_end && isdigit(*(a_it + 1))) {
        a_it++;
    }

    if (u_it == u_end || a_it == a_end) {
        return;
    }

    auto cp = utf8::next(u_it, u_end);

    a_it += asciiLettersPerCodepoint(cp);

    while (u_it != u_end && cursorSkipsCodepoint(utf8::peek_next(u_it, u_end))) {
        cp = utf8::next(u_it, u_end);
        a_it += asciiLettersPerCodepoint(cp);
    }

    while (a_it != a_end && isdigit(*a_it)) {
        a_it++;
    }
}

template <typename iter_t>
void ParallelPrior(iter_t &a_it, iter_t &a_begin, iter_t &u_it, iter_t &u_begin) {
    if (a_it == a_begin || u_it == u_begin) {
        return;
    }

    if (a_it != a_begin && isdigit(a_it[-1])) {
        do {
            a_it--;
        } while (a_it != a_begin && isdigit(a_it[-1]));

        if (a_it == a_begin || u_it == u_begin) {
            return;
        }
    }
    auto cp = utf8::unchecked::prior(u_it);

    while (u_it != u_begin && cursorSkipsCodepoint(cp)) {
        a_it -= asciiLettersPerCodepoint(cp);
        cp = utf8::unchecked::prior(u_it);
    }

    if (cp == U32_TK && std::distance(a_begin, a_it) > 1 && a_it[-1] == '-' && a_it[-2] == '-') {
        a_it -= 2;
    } else {
        a_it -= asciiLettersPerCodepoint(cp);
    }
}

} // namespace khiin::engine
