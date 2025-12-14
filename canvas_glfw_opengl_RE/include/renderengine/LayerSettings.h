#ifndef _LAYER_SETTINGS_H_
#define _LAYER_SETTINGS_H_

#include <vector>

#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkM44.h"

#include "ui/FloatRect.h"
#include "ui/Dataspace.h"
#include "ui/ShadowSettings.h"
#include "ui/BlurRegion.h"
#include "gui/BorderSettings.h"

namespace renderengine {

struct Buffer {
    // Buffer containing the image that we will render.
    // If buffer == nullptr, then the rest of the fields in this struct will be
    // ignored.
    sk_sp<SkImage> buffer = nullptr;

    // Fence that will fire when the buffer is ready to be bound.
    // sp<Fence> fence = nullptr;

    // Whether to use filtering when rendering the texture.
    bool useTextureFiltering = false;

    // Transform matrix to apply to texture coordinates.
    SkM44 textureTransform = SkM44();

    // Whether to use pre-multiplied alpha.
    bool usePremultipliedAlpha = true;

    // Override flag that alpha for each pixel in the buffer *must* be 1.0.
    // LayerSettings::alpha is still used if isOpaque==true - this flag only
    // overrides the alpha channel of the buffer.
    bool isOpaque = false;

    float maxLuminanceNits = 0.0;
};

struct Geometry {
    // Boundaries of the layer.
    ui::FloatRect boundaries = ui::FloatRect();

    // Boundaries of the layer before transparent region hint is subtracted.
    // Effects like shadows and outline ignore the transparent region hint.
    ui::FloatRect originalBounds = ui::FloatRect();

    // Transform matrix to apply to mesh coordinates.
    SkM44 positionTransform = SkM44();

    // Radius of rounded corners, if greater than 0. Otherwise, this layer's
    // corners are not rounded.
    // Having corner radius will force GPU composition on the layer and its children, drawing it
    // with a special shader. The shader will receive the radius and the crop rectangle as input,
    // modifying the opacity of the destination texture, multiplying it by a number between 0 and 1.
    // We query Layer#getRoundedCornerState() to retrieve the radius as well as the rounded crop
    // rectangle to figure out how to apply the radius for this layer. The crop rectangle will be
    // in local layer coordinate space, so we have to take the layer transform into account when
    // walking up the tree.
    SkV2 roundedCornersRadius = SkV2();

    // Rectangle within which corners will be rounded.
    ui::FloatRect roundedCornersCrop = ui::FloatRect();
};

struct PixelSource {
    // Source buffer
    Buffer buffer = Buffer();

    // The solid color with which to fill the layer.
    // This should only be populated if we don't render from an application
    // buffer.
    SkV3 solidColor = SkV3();
};

struct LayerSettings {
    // Geometry information
    Geometry geometry = Geometry();

    // Source pixels for this layer.
    PixelSource source = PixelSource();

    // Alpha option to blend with the source pixels
    float alpha = 0.0f;

    // Color space describing how the source pixels should be interpreted.
    ui::Dataspace sourceDataspace = ui::Dataspace::HAL_DATASPACE_UNKNOWN;

    // Additional layer-specific color transform to be applied before the global
    // transform.
    SkM44 colorTransform = SkM44();

    // True if blending will be forced to be disabled.
    bool disableBlending = false;

    // If true, then this layer casts a shadow and/or blurs behind it, but it does
    // not otherwise draw any of the layer's other contents.
    bool skipContentDraw = false;

    ui::ShadowSettings shadow;

    gui::BorderSettings borderSettings;

    int backgroundBlurRadius = 0;

    std::vector<BlurRegion> blurRegions;

    // Transform matrix used to convert the blurRegions geometry into the same
    // coordinate space as LayerSettings.geometry
    SkM44 blurRegionTransform = SkM44();

    // StretchEffect stretchEffect;
    // EdgeExtensionEffect edgeExtensionEffect;

    // Name associated with the layer for debugging purposes.
    std::string name;

    // Luminance of the white point for this layer. Used for linear dimming.
    // Individual layers will be dimmed by (whitePointNits / maxWhitePoint).
    // If white point nits are unknown, then this layer is assumed to have the
    // same luminance as the brightest layer in the scene.
    float whitePointNits = -1.f;

    // std::shared_ptr<gui::DisplayLuts> luts;
};


}

#endif  // _LAYER_SETTINGS_H_