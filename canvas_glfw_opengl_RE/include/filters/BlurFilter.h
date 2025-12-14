#ifndef _BLUR_FILTER_H_
#define _BLUR_FILTER_H_

#include "skia/include/core/SkImage.h"
#include "skia/include/core/SkCanvas.h"
#include "skia/include/effects/SkRuntimeEffect.h"
#include "skia/include/core/SkSurface.h"

#include "skia/compat/SkiaGpuContext.h"

namespace renderengine {
namespace skia {

class BlurFilter {
public:
    // Downsample FBO to improve performance
    static constexpr float kInputScale = 0.25f;
    // Downsample scale factor used to improve performance
    static constexpr float kInverseInputScale = 1.0f / kInputScale;

    explicit BlurFilter(float maxCrossFadeRadius = 10.0f);
    virtual ~BlurFilter(){}

    // Execute blur, saving it to a texture
    virtual sk_sp<SkImage> generate(SkiaGpuContext* context, const uint32_t radius,
                                    const sk_sp<SkImage> blurInput,
                                    const SkRect& blurRect) const = 0;

    /**
     * Draw the blurred content (from the generate method) into the canvas.
     * @param canvas is the destination/output for the blur
     * @param effectRegion the RoundRect in canvas coordinates that determines the blur coverage
     * @param blurRadius radius of the blur used to determine the intensity of the crossfade effect
     * @param blurAlpha alpha value applied to the effectRegion when the blur is drawn
     * @param blurRect bounds of the blurredImage translated into canvas coordinates
     * @param blurredImage down-sampled blurred content that was produced by the generate() method
     * @param input original unblurred input that is used to crossfade with the blurredImage
     */
    void drawBlurRegion(SkCanvas* canvas, const SkRRect& effectRegion,
                                const uint32_t blurRadius, const float blurAlpha,
                                const SkRect& blurRect, sk_sp<SkImage> blurredImage,
                                sk_sp<SkImage> input);

    float getMaxCrossFadeRadius() const;

private:
    // To avoid downscaling artifacts, we interpolate the blurred fbo with the full composited
    // image, up to this radius.
    const float mMaxCrossFadeRadius;

    // Optional blend used for crossfade only if mMaxCrossFadeRadius > 0
    const sk_sp<SkRuntimeEffect> mMixEffect;
};


} // namespace skia
} // namespace renderengine

#endif  // end _BLUR_FILTER_H_