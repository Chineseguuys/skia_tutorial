#ifndef _SHADOW_SETTINGS_H_
#define _SHADOW_SETTINGS_H_

#include "FloatRect.h"
#include "include/core/SkM44.h"

namespace ui {

struct ShadowSettings {
    // Boundaries of the shadow.
    FloatRect boundaries = FloatRect();

    // Color to the ambient shadow. The alpha is premultiplied.
    SkV4 ambientColor = SkV4();

    // Color to the spot shadow. The alpha is premultiplied. The position of the spot shadow
    // depends on the light position.
    SkV4 spotColor = SkV4();

    // Position of the light source used to cast the spot shadow.
    SkV3 lightPos = SkV3();

    // Radius of the spot light source. Smaller radius will have sharper edges,
    // larger radius will have softer shadows
    float lightRadius = 0.f;

    // Length of the cast shadow. If length is <= 0.f no shadows will be drawn.
    float length = 0.f;

    // If true fill in the casting layer is translucent and the shadow needs to fill the bounds.
    // Otherwise the shadow will only be drawn around the edges of the casting layer.
    bool casterIsTranslucent = false;
};

static inline bool operator==(const ShadowSettings& lhs, const ShadowSettings& rhs) {
    return lhs.boundaries == rhs.boundaries && lhs.ambientColor == rhs.ambientColor &&
            lhs.spotColor == rhs.spotColor && lhs.lightPos == rhs.lightPos &&
            lhs.lightRadius == rhs.lightRadius && lhs.length == rhs.length &&
            lhs.casterIsTranslucent == rhs.casterIsTranslucent;
}

static inline bool operator!=(const ShadowSettings& lhs, const ShadowSettings& rhs) {
    return !(operator==(lhs, rhs));
}


}

#endif  // end _SHADOW_SETTINGS_H_