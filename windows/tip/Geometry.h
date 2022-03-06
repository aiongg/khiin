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
    Rect(Point origin, int width, int height) : origin_(origin), width_(width), height_(height){};

    inline int left() {
        return origin_.x;
    }
    inline int right() {
        return origin_.x + width_;
    }
    inline int top() {
        return origin_.y;
    }
    inline int bottom() {
        return origin_.y + height_;
    }
    inline int width() {
        return width_;
    }
    inline int height() {
        return height_;
    }
    inline float leftf() {
        return static_cast<float>(origin_.x);
    }
    inline float rightf() {
        return static_cast<float>(origin_.x + width_);
    }
    inline float topf() {
        return static_cast<float>(origin_.y);
    }
    inline float bottomf() {
        return static_cast<float>(origin_.y + height_);
    }
    inline float widthf() {
        return static_cast<float>(width_);
    }
    inline float heightf() {
        return static_cast<float>(height_);
    }
    inline Size size() {
        return Size{width_, height_};
    }
    inline bool Hit(Point const &p) {
        return p.x >= left() && p.x <= right() && p.x >= top() && p.x <= bottom();
    }
    inline Point origin() {
        return origin_;
    }

  private:
    Point origin_; // Top-left
    int width_;
    int height_;
};

} // namespace khiin::geometry
