#ifndef _FLOAT_RECT_H_
#define _FLOAT_RECT_H_

#include <iostream>

namespace ui {

class FloatRect {
public:
    FloatRect() = default;
    constexpr FloatRect(float _left, float _top, float _right, float _bottom)
      : left(_left), top(_top), right(_right), bottom(_bottom) {}

    float getWidth() const { return right - left; }
    float getHeight() const { return bottom - top; }

    FloatRect intersect(const FloatRect& other) const {
        FloatRect intersection = {
            // Inline to avoid tromping on other min/max defines or adding a
            // dependency on STL
            (left > other.left) ? left : other.left,
            (top > other.top) ? top : other.top,
            (right < other.right) ? right : other.right,
            (bottom < other.bottom) ? bottom : other.bottom
        };
        if (intersection.getWidth() < 0 || intersection.getHeight() < 0) {
            return {0, 0, 0, 0};
        }
        return intersection;
    }

    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    constexpr bool isEmpty() const { return !(left < right && top < bottom); }

    // a valid rectangle has a non negative width and height
    inline bool isValid() const { return (getWidth() >= 0) && (getHeight() >= 0); }
};

inline bool operator==(const FloatRect& a, const FloatRect& b) {
    return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
}

static inline void PrintTo(const FloatRect& rect, ::std::ostream* os) {
    *os << "FloatRect(" << rect.left << ", " << rect.top << ", " << rect.right << ", "
        << rect.bottom << ")";
}

} // namespace ui

#endif  // end _LAYER_SETTINGS_H_