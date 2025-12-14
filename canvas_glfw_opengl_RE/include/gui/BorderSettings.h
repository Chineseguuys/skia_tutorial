#ifndef _BORDER_SETTINGS_H_
#define _BORDER_SETTINGS_H_

#include "include/core/SkColor.h"
namespace gui {

struct BorderSettings {
    // Width of the border in pixels.
    float strokeWidth = 0.0f;

    // Color of the border. The alpha is premultiplied.
    SkColor color = SK_ColorTRANSPARENT;
};

} // namespace gui

#endif  // end _BORDER_SETTINGS_H_