#ifndef _BLUR_REGION_H_
#define _BLUR_REGION_H_

#include <cstdint>
#include <iostream>

struct BlurRegion {
    uint32_t blurRadius;
    float cornerRadiusTL;
    float cornerRadiusTR;
    float cornerRadiusBL;
    float cornerRadiusBR;
    float alpha;
    int left;
    int top;
    int right;
    int bottom;

    inline bool operator==(const BlurRegion& other) const {
        return blurRadius == other.blurRadius && cornerRadiusTL == other.cornerRadiusTL &&
                cornerRadiusTR == other.cornerRadiusTR && cornerRadiusBL == other.cornerRadiusBL &&
                cornerRadiusBR == other.cornerRadiusBR && alpha == other.alpha &&
                left == other.left && top == other.top && right == other.right &&
                bottom == other.bottom;
    }

    inline bool operator!=(const BlurRegion& other) const { return !(*this == other); }
};

static inline void PrintTo(const BlurRegion& blurRegion, ::std::ostream* os) {
    *os << "BlurRegion {";
    *os << "\n    .blurRadius = " << blurRegion.blurRadius;
    *os << "\n    .cornerRadiusTL = " << blurRegion.cornerRadiusTL;
    *os << "\n    .cornerRadiusTR = " << blurRegion.cornerRadiusTR;
    *os << "\n    .cornerRadiusBL = " << blurRegion.cornerRadiusBL;
    *os << "\n    .cornerRadiusBR = " << blurRegion.cornerRadiusBR;
    *os << "\n    .alpha = " << blurRegion.alpha;
    *os << "\n    .left = " << blurRegion.left;
    *os << "\n    .top = " << blurRegion.top;
    *os << "\n    .right = " << blurRegion.right;
    *os << "\n    .bottom = " << blurRegion.bottom;
    *os << "\n}";
}

#endif //_BLUR_REGION_H_