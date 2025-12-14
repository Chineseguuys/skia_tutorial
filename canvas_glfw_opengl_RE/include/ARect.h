#ifndef _ARECT_H_
#define _ARECT_H_


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Rectangular window area.
 *
 * This is the NDK equivalent of the android.graphics.Rect class in Java. It is
 * used with {@link ANativeActivityCallbacks::onContentRectChanged} event
 * callback and the ANativeWindow_lock() function.
 *
 * In a valid ARect, left <= right and top <= bottom. ARect with left=0, top=10,
 * right=1, bottom=11 contains only one pixel at x=0, y=10.
 */
typedef struct ARect {
#ifdef __cplusplus
    typedef int32_t value_type;
#endif
    /// Minimum X coordinate of the rectangle.
    int32_t left;
    /// Minimum Y coordinate of the rectangle.
    int32_t top;
    /// Maximum X coordinate of the rectangle.
    int32_t right;
    /// Maximum Y coordinate of the rectangle.
    int32_t bottom;
} ARect;

#ifdef __cplusplus
}
#endif

#endif // end _ARECT_H_