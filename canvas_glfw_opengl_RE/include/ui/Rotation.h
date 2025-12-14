#ifndef _ROTATION_H_
#define _ROTATION_H_

#include <type_traits>


typedef enum {
    HAL_TRANSFORM_FLIP_H = 1,   // (1 << 0)
    HAL_TRANSFORM_FLIP_V = 2,   // (1 << 1)
    HAL_TRANSFORM_ROT_90 = 4,   // (1 << 2)
    HAL_TRANSFORM_ROT_180 = 3,  // (FLIP_H | FLIP_V)
    HAL_TRANSFORM_ROT_270 = 7,  // ((FLIP_H | FLIP_V) | ROT_90)
} android_transform_t;

namespace ui {

enum class Rotation {
    Rotation0 = 0,
    Rotation90 = 1,
    Rotation180 = 2,
    Rotation270 = 3,

    ftl_last = Rotation270
};

// Equivalent to Surface.java constants.
constexpr auto ROTATION_0 = Rotation::Rotation0;
constexpr auto ROTATION_90 = Rotation::Rotation90;
constexpr auto ROTATION_180 = Rotation::Rotation180;
constexpr auto ROTATION_270 = Rotation::Rotation270;

constexpr auto toRotation(std::underlying_type_t<Rotation> rotation) {
    return static_cast<Rotation>(rotation);
}

constexpr auto toRotationInt(Rotation rotation) {
    return static_cast<std::underlying_type_t<Rotation>>(rotation);
}

constexpr Rotation operator+(Rotation lhs, Rotation rhs) {
    constexpr auto N = toRotationInt(ROTATION_270) + 1;
    return toRotation((toRotationInt(lhs) + toRotationInt(rhs)) % N);
}

constexpr Rotation operator-(Rotation lhs, Rotation rhs) {
    constexpr auto N = toRotationInt(ROTATION_270) + 1;
    return toRotation((N + toRotationInt(lhs) - toRotationInt(rhs)) % N);
}

constexpr Rotation operator-(Rotation rotation) {
    return ROTATION_0 - rotation;
}

constexpr const char* toCString(Rotation rotation) {
    switch (rotation) {
        case ROTATION_0:
            return "ROTATION_0";
        case ROTATION_90:
            return "ROTATION_90";
        case ROTATION_180:
            return "ROTATION_180";
        case ROTATION_270:
            return "ROTATION_270";
    }
}

}


#endif // end _ROTATION_H_