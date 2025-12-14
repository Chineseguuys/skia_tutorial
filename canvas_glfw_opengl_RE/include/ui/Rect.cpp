#include "Rect.h"
#include <cstdio>
#include "StringPrintf.h"

namespace ui {

const Rect Rect::INVALID_RECT{0, 0, -1, -1};
const Rect Rect::EMPTY_RECT{0, 0, 0, 0};

static inline int32_t min(int32_t a, int32_t b) {
    return (a < b) ? a : b;
}

static inline int32_t max(int32_t a, int32_t b) {
    return (a > b) ? a : b;
}

void Rect::makeInvalid() {
    left = 0;
    top = 0;
    right = -1;
    bottom = -1;
}

bool Rect::operator <(const Rect& rhs) const {
    if (top < rhs.top) {
        return true;
    } else if (top == rhs.top) {
        if (left < rhs.left) {
            return true;
        } else if (left == rhs.left) {
            if (bottom < rhs.bottom) {
                return true;
            } else if (bottom == rhs.bottom) {
                if (right < rhs.right) {
                    return true;
                }
            }
        }
    }
    return false;
}

Rect& Rect::offsetTo(int32_t x, int32_t y) {
    right -= left - x;
    bottom -= top - y;
    left = x;
    top = y;
    return *this;
}

Rect& Rect::offsetBy(int32_t x, int32_t y) {
    left += x;
    top += y;
    right += x;
    bottom += y;
    return *this;
}

Rect& Rect::inset(int32_t _left, int32_t _top, int32_t _right, int32_t _bottom) {
    this->left += _left;
    this->top += _top;
    this->right -= _right;
    this->bottom -= _bottom;
    return *this;
}

const Rect Rect::operator +(const Point& rhs) const {
    const Rect result(left + rhs.x, top + rhs.y, right + rhs.x, bottom + rhs.y);
    return result;
}

const Rect Rect::operator -(const Point& rhs) const {
    const Rect result(left - rhs.x, top - rhs.y, right - rhs.x, bottom - rhs.y);
    return result;
}

bool Rect::intersect(const Rect& with, Rect* result) const {
    result->left = max(left, with.left);
    result->top = max(top, with.top);
    result->right = min(right, with.right);
    result->bottom = min(bottom, with.bottom);
    return !(result->isEmpty());
}

Rect Rect::transform(uint32_t xform, int32_t width, int32_t height) const {
    Rect result(*this);
    if (xform & HAL_TRANSFORM_FLIP_H) {
        result = Rect(width - result.right, result.top, width - result.left,
                result.bottom);
    }
    if (xform & HAL_TRANSFORM_FLIP_V) {
        result = Rect(result.left, height - result.bottom, result.right,
                height - result.top);
    }
    if (xform & HAL_TRANSFORM_ROT_90) {
        int left = height - result.bottom;
        int top = result.left;
        int right = height - result.top;
        int bottom = result.right;
        result = Rect(left, top, right, bottom);
    }
    return result;
}

Rect Rect::reduce(const Rect& exclude) const {
    Rect result(Rect::EMPTY_RECT);

    uint32_t mask = 0;
    mask |= (exclude.left   > left)   ? 1 : 0;
    mask |= (exclude.top    > top)    ? 2 : 0;
    mask |= (exclude.right  < right)  ? 4 : 0;
    mask |= (exclude.bottom < bottom) ? 8 : 0;

    if (mask == 0) {
        // crop entirely covers us
        result.clear();
    } else {
        result = *this;
        if (!(mask & (mask - 1))) {
            // power-of-2, i.e.: just one bit is set
            if (mask & 1) {
                result.right = min(result.right, exclude.left);
            } else if (mask & 2) {
                result.bottom = min(result.bottom, exclude.top);
            } else if (mask & 4) {
                result.left = max(result.left, exclude.right);
            } else if (mask & 8) {
                result.top = max(result.top, exclude.bottom);
            }
        }
    }

    return result;
}

std::string to_string(const Rect& rect) {
    return ::base::StringPrintf("Rect(%d, %d, %d, %d)", rect.left, rect.top, rect.right,
                                       rect.bottom);
}

void PrintTo(const Rect& rect, ::std::ostream* os) {
    *os << to_string(rect);
}

} // namespace ui