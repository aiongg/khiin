#pragma once

namespace khiin::geometry {

struct Point {
    int x = 0;
    int y = 0;
};

struct Size {
    int width = 0;
    int height = 0;
};

struct Rect {
    Rect() = default;
    Rect(Point origin, int width, int height) : origin(origin), width(width), height(height){};

    inline int left() {
        return origin.x;
    }
    inline int right() {
        return origin.x + width;
    }
    inline int top() {
        return origin.y;
    }
    inline int bottom() {
        return origin.y + height;
    }
    inline Size size() {
        return Size{width, height};
    }
    inline bool Hit(Point const &p) {
        return p.x >= left() && p.x <= right() && p.x >= top() && p.x <= bottom();
    }

    Point origin; // Top-left
    int width;
    int height;
};

} // namespace khiin::geometry
