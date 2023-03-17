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
    Rect(Point origin, int width, int height) : o(origin), w(width), h(height){};

    inline int left() const {
        return o.x;
    }
    inline int right() const {
        return o.x + w;
    }
    inline int top() const {
        return o.y;
    }
    inline int bottom() const {
        return o.y + h;
    }
    inline int width() const {
        return w;
    }
    inline int height() const {
        return h;
    }
    inline float leftf() const {
        return static_cast<float>(o.x);
    }
    inline float rightf() const {
        return static_cast<float>(o.x + w);
    }
    inline float topf() const {
        return static_cast<float>(o.y);
    }
    inline float bottomf() const {
        return static_cast<float>(o.y + h);
    }
    inline float widthf() const {
        return static_cast<float>(w);
    }
    inline float heightf() const {
        return static_cast<float>(h);
    }
    inline Size size() const {
        return Size{w, h};
    }
    inline Point origin() const {
        return o;
    }
    inline Point center() const {
        return Point{o.x + w / 2, o.y + h / 2};
    }
    inline bool Hit(Point const &pt) const {
        return pt.x >= left() && pt.x <= right() && pt.y >= top() && pt.y <= bottom();
    }

  private:
    Point o; // Top-left
    int w = 0;
    int h = 0;
};

} // namespace khiin::geometry
