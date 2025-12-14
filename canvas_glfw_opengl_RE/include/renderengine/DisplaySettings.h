#ifndef _DISPLAY_SETTINGS_H_
#define _DISPLAY_SETTINGS_H_

#include <string>
#include <cstdint>
#include "math/mat4.h"
#include "ui/Rect.h"
#include "ui/Dataspace.h"
#include "ui/Transform.h"

namespace renderengine {

// DisplaySettings contains the settings that are applicable when drawing all
// layers for a given display.
struct DisplaySettings {
    // A string containing the name of the display, along with its id, if it has
    // one.
    std::string namePlusId;

    // Rectangle describing the physical display. We will project from the
    // logical clip onto this rectangle.
    ui::Rect physicalDisplay = ui::Rect::INVALID_RECT;

    // Rectangle bounded by the x,y- clipping planes in the logical display, so
    // that the orthographic projection matrix can be computed. When
    // constructing this matrix, z-coordinate bound are assumed to be at z=0 and
    // z=1.
    ui::Rect clip = ui::Rect::INVALID_RECT;

    // Maximum luminance pulled from the display's HDR capabilities.
    float maxLuminance = 1.0f;

    // Current luminance of the display
    float currentLuminanceNits = -1.f;

    // Output dataspace that will be populated if wide color gamut is used, or
    // DataSpace::UNKNOWN otherwise.
    ui::Dataspace outputDataspace = ui::Dataspace::HAL_DATASPACE_UNKNOWN;

    // Additional color transform to apply after transforming to the output
    // dataspace, in non-linear space.
    mat4 colorTransform = mat4();

    // If true, and colorTransform is non-identity, most client draw calls can
    // ignore it. Some draws (e.g. screen decorations) may need it, though.
    bool deviceHandlesColorTransform = false;

    // An additional orientation flag to be applied after clipping the output.
    // By way of example, this may be used for supporting fullscreen screenshot
    // capture of a device in landscape while the buffer is in portrait
    // orientation.
    uint32_t orientation = ui::Transform::ROT_0;

    // Target luminance of the display. -1f if unknown.
    // All layers will be dimmed by (max(layer white points) / targetLuminanceNits).
    // If the target luminance is unknown, then no display-level dimming occurs.
    float targetLuminanceNits = -1.f;

    // Configures when dimming should be applied for each layer.
    // aidl::android::hardware::graphics::composer3::DimmingStage dimmingStage =
    //         aidl::android::hardware::graphics::composer3::DimmingStage::NONE;

    // Configures the rendering intent of the output display. This is used for tonemapping.
    // aidl::android::hardware::graphics::composer3::RenderIntent renderIntent =
    //         aidl::android::hardware::graphics::composer3::RenderIntent::TONE_MAP_COLORIMETRIC;

    // Tonemapping strategy to use for each layer. This is only used for tonemapping HDR source
    // content
    enum class TonemapStrategy {
        // Use a tonemapper defined by libtonemap. This may be OEM-defined as of Android 13, aka
        // undefined.
        // This is typically a global tonemapper, designed to match what is on screen.
        Libtonemap,
        // Use a local tonemapper. Because local tonemapping uses large intermediate allocations,
        // this
        // method is primarily recommended for infrequent rendering that does not need to exactly
        // match
        // pixels that are on-screen.
        Local,
    };
    TonemapStrategy tonemapStrategy = TonemapStrategy::Libtonemap;

    // For now, meaningful primarily when the TonemappingStrategy is Local
    float targetHdrSdrRatio = 1.f;
};

} // namespace renderengine


#endif // end _DISPLAY_SETTINGS_H_