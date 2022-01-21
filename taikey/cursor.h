#pragma once

#include <string>
#include <vector>

namespace TaiKey {

//class SynchronizedBuffer;
//struct Segment;
//using SegmentIter = std::vector<Segment>::iterator;


//class Cursor {
//  public:
//    using u32_iter = std::u32string::iterator;
//    using self_type = Cursor;
//    using value_type = std::pair<u32_iter, u32_iter>;
//    using reference = std::pair<u32_iter &, u32_iter &>;
//    using pointer = std::pair<u32_iter *, u32_iter *>;
//    using difference_type = std::pair<ptrdiff_t, ptrdiff_t>;
//
//    Cursor(u32_iter raw_begin, u32_iter raw_iter, u32_iter raw_end, u32_iter display_begin,
//           u32_iter display_iter, u32_iter display_end)
//        : rb(raw_begin), ri(raw_iter), re(raw_end), db(display_begin),
//          di(display_iter), de(display_end){};
//
//    self_type operator++() {
//        self_type c = *this;
//        next();
//        return c;
//    };
//
//    self_type operator++(int) {
//        next();
//        return *this;
//    };
//
//    self_type operator--() {
//        self_type c = *this;
//        prior();
//        return c;
//    };
//
//    self_type operator--(int) {
//        prior();
//        return *this;
//    };
//
//    reference operator*() {
//        return std::make_pair(ri, di);
//    };
//
//    pointer operator->() {
//        return std::make_pair(&ri, &di);
//    };
//
//    bool operator==(const self_type &rhs) {
//        return ri == rhs.ri && di == rhs.di;
//    };
//
//    bool operator!=(const self_type &rhs) {
//        return ri != rhs.ri || di != rhs.di;
//    };
//
//  private:
//    void next();
//    void prior();
//    bool atEnd() { return rAtEnd() && dAtEnd(); };
//    bool rAtEnd() { return ri == re; };
//    bool dAtEnd() { return di == de; };
//    bool rAtBack() { return ri == re - 1; };
//
//    u32_iter rb;
//    u32_iter ri;
//    u32_iter re;
//
//    u32_iter db;
//    u32_iter di;
//    u32_iter de;
//};

} // namespace TaiKey